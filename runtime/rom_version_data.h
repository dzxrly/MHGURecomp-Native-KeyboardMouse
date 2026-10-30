// Identity metadata from the supported, unmodified Switch MHGU 1.4.0 dump.
// Resource fingerprints cover selected base resources, not every file in romfs.
#pragma once
#include <stdint.h>
typedef struct { const char *name, *build_id; } RomModule;
typedef struct { const char *name; uint64_t size; const char *sha256; } RomResource;
static const RomModule rom_modules[] = {
    {"main", "fb08f1d20fd1204fe7070672777f869b00000000000000000000000000000000"},
    {"sdk", "cb8e7f9a74a4a854ff013489ecc1e8b0def23a75000000000000000000000000"},
};
static const RomResource rom_resources[] = {
    {"nativeNX/system/app_shader/AppShaderPackage.mfx", 1300533, "2b80892da570f42b38dfe46b31234503af3d593eb634da10109c37534ac36627"},
    {"nativeNX/table/activityData.atd", 10128, "1696358c021dec3cbd73742db9176160f4e04a62723bf86bd2f9e6bc4c0a7075"},
    {"nativeNX/eng/table/TitleMsg_eng.gmd", 8919, "e994ade4181b6e8a4095054b307d3c4d42e7ddc308f1fe6f059648e5a38caa23"},
    {"nativeNX/chS/table/TitleMsg_chS.gmd", 9242, "8128eb9d69930cc166a1dbff598f30a7c880becee147b1fa6a27c8ba18d4fa8b"},
    {"nativeNX/chT/table/TitleMsg_chT.gmd", 9444, "c670be3417f369542d8c202a8c8fc33d0548dd25e2389eca1778387a178e6ef4"},
    {"nativeNX/fre/table/TitleMsg_fre.gmd", 9866, "c7e84f693a4c6bb4c54dad0b5646ec7371a016429aa2abccf00b1e5d2ecdefc2"},
    {"nativeNX/ger/table/TitleMsg_ger.gmd", 9975, "cfd982cf925216a36b38e5be9d380dc80bba4aab7ea62699118ddaf7de5deef5"},
    {"nativeNX/ita/table/TitleMsg_ita.gmd", 9635, "f9500b111d3ad1d32bd5a7977d5bf1035fa6a1e27f4e927b56aa7abfe175fd0a"},
    {"nativeNX/spa/table/TitleMsg_spa.gmd", 9640, "3dca668e780aa172b466395161d98296aabcd33a4f7024ad3cfec8d1223acc50"},
};
