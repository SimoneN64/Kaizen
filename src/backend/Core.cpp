#include <Core.hpp>
#include <Scheduler.hpp>
#include <ParallelRDPWrapper.hpp>

namespace n64 {
Core::Core(ParallelRDP& parallel) : cpu(std::make_unique<Interpreter>(parallel)) {}

void Core::Stop() {
  render = false;
  pause = true;
  romLoaded = false;
  cpu->Reset();
  cpu->GetMem().Reset();
}

bool Core::LoadTAS(const fs::path &path) {
  return cpu->GetMem().mmio.si.pif.movie.Load(path);
}

void Core::LoadROM(const std::string& rom_) {
  pause = true;
  rom = rom_;
  cpu->Reset();
  romLoaded = true;

  std::string archive_types[] = {".zip",".7z",".rar",".tar"};

  auto extension = fs::path(rom).extension().string();
  bool isArchive = std::any_of(std::begin(archive_types), std::end(archive_types), [&extension](const auto& e) {
    return e == extension;
  });

  cpu->GetMem().LoadROM(isArchive, rom);
  GameDB::match(cpu->GetMem());
  cpu->GetMem().mmio.vi.isPal = cpu->GetMem().IsROMPAL();
  cpu->GetMem().mmio.si.pif.InitDevices(cpu->GetMem().saveType);
  cpu->GetMem().mmio.si.pif.mempakPath = rom;
  cpu->GetMem().mmio.si.pif.LoadEeprom(cpu->GetMem().saveType, rom);
  cpu->GetMem().flash.Load(cpu->GetMem().saveType, rom);
  cpu->GetMem().LoadSRAM(cpu->GetMem().saveType, rom);
  cpu->GetMem().mmio.si.pif.Execute();
  pause = false;
  render = true;
}

void Core::Run(float volumeL, float volumeR) {
  Mem& mem = cpu->GetMem();
  MMIO& mmio = mem.mmio;
  Registers& regs = cpu->GetRegs();

  for (int field = 0; field < mmio.vi.numFields; field++) {
    u32 frameCycles = 0;
    for (int i = 0; i < mmio.vi.numHalflines; i++) {
      mmio.vi.current = (i << 1) + field;

      if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
        mmio.mi.InterruptRaise(MI::Interrupt::VI);
      }

      for(; cycles < mem.mmio.vi.cyclesPerHalfline; cycles++, frameCycles++) {
        u32 taken = cpu->Step();
        taken += regs.PopStalledCycles();
        static u32 cpuSteps = 0;
        cpuSteps += taken;
        if(mmio.rsp.spStatus.halt) {
          cpuSteps = 0;
          mmio.rsp.steps = 0;
        } else {
          while(cpuSteps > 2) {
            mmio.rsp.steps += 2;
            cpuSteps -= 3;
          }

          while(mmio.rsp.steps > 0) {
            mmio.rsp.steps--;
            mmio.rsp.Step();
          }
        }

        cycles += taken;
        frameCycles += taken;
        scheduler.Tick(taken, mem);
      }

      cycles -= mmio.vi.cyclesPerHalfline;
    }

    if ((mmio.vi.current & 0x3FE) == mmio.vi.intr) {
      mmio.mi.InterruptRaise(MI::Interrupt::VI);
    }

    mmio.ai.Step(frameCycles, volumeL, volumeR);
    scheduler.Tick(frameCycles, mem);
  }
}

void Core::Serialize() {
  auto sMEM = cpu->GetMem().Serialize();
  auto sCPU = cpu->Serialize();
  auto sVER = std::vector<u8>{KAIZEN_VERSION >> 8, KAIZEN_VERSION >> 4, KAIZEN_VERSION & 0xFF};
  memSize = sMEM.size();
  cpuSize = sCPU.size();
  verSize = sVER.size();
  serialized[slot].insert(serialized[slot].begin(), sVER.begin(), sVER.end());
  serialized[slot].insert(serialized[slot].end(), sMEM.begin(), sMEM.end());
  serialized[slot].insert(serialized[slot].end(), sCPU.begin(), sCPU.end());
}

void Core::Deserialize() {
  std::vector<u8> dVER(serialized[slot].begin(), serialized[slot].begin() + verSize);
  if(dVER[0] != (KAIZEN_VERSION >> 8)
  || dVER[1] != (KAIZEN_VERSION >> 4)
  || dVER[2] != (KAIZEN_VERSION & 0xFF)) {
    Util::panic("PROBLEMI!");
  }

  cpu->GetMem().Deserialize(std::vector<u8>(serialized[slot].begin() + verSize, serialized[slot].begin() + verSize + memSize));
  cpu->Deserialize(std::vector<u8>(serialized[slot].begin() + verSize + memSize, serialized[slot].begin() + verSize + memSize + cpuSize));
  serialized[slot].erase(serialized[slot].begin(), serialized[slot].end());
}
}
