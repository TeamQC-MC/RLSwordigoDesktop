#include "mod_loader.h"
#include <iostream>
#include <vector>
#include <stdint.h>
#include "loader/elf_loader.h"
#include "jni/jni_layer.h"
#include "jni/jni_bridge.h"
#include "platform/emulator.h"
#include "platform/display.h"
#include "android/asset_manager.h"
#include <cstring>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <sys/stat.h>
#include <SDL2/SDL.h>

#include "platform/gui.h"

// Externs from main.cpp to access the shared infrastructure
extern uint8_t* g_guest_memory;
extern const uint32_t GUEST_MEM_SIZE;
extern const uint32_t GUEST_GLOBALS_BASE;
extern so_module g_main_mod;
extern ElfLoader* g_loader;
extern JniBridge g_bridge;
extern Emulator* g_emulator;
extern GuiRenderer g_gui;
extern std::string g_save_dir;
extern std::string g_cache_dir;
extern bool g_display_active;
extern int g_win_w;
extern int g_win_h;
extern FrameStats g_frame_stats;

extern std::string get_data_path(const std::string& relative_path);
extern void call_handle_touch_event(uint32_t addr, uint32_t env, uint32_t obj, int action, int id, double time_val, float x, float y, float old_x, float old_y, int tap_count);
extern uint32_t setup_jni_env(uint8_t* memory);
extern void start_game_loop(uint32_t env_ptr); // We'll need to export this from main.cpp

// MODDED INSTANCE MODULES
so_module g_gloss_mod;
so_module g_mini_mod;

// --- GlossHook & SwMini Compatibility Stubs ---
void bridge_GlossInit(void* emu_ptr) {
    Emulator* emu = (Emulator*)emu_ptr;
    std::cout << "[Gloss] GlossInit called (InitLinker=" << emu->get_reg(0) << ")" << std::endl;
    emu->set_reg(0, 1);
}

void bridge_GlossEnableLog(void* emu_ptr) {
    Emulator* emu = (Emulator*)emu_ptr;
    std::cout << "[Gloss] GlossEnableLog called (" << emu->get_reg(0) << ")" << std::endl;
}

void bridge_GlossHookAddr(void* emu_ptr) {
    Emulator* emu = (Emulator*)emu_ptr;
    uint32_t addr = emu->get_reg(0);
    uint32_t new_func = emu->get_reg(1);
    uint32_t orig_ptr = emu->get_reg(2);
    
    std::cout << "[Gloss] GlossHookAddr: 0x" << std::hex << addr << " -> 0x" << new_func << std::dec << std::endl;
    
    // In our emulator, we can implement hooks by simply writing a branch to the bridge or another guest address.
    // However, SwMini expects GlossHook to return a 'stub' that calls the original.
    // For now, let's just log it. Real hooking would require more complex trampoline generation.
    // libmini.so usually hooks Engine functions.
    
    emu->set_reg(0, addr); // return original address as stub for now (dangerous but might work if it doesn't call orig)
}

void bridge_WriteMemory(void* emu_ptr) {
    Emulator* emu = (Emulator*)emu_ptr;
    uint32_t addr = emu->get_reg(0);
    uint32_t data_ptr = emu->get_reg(1);
    uint32_t size = emu->get_reg(2);
    
    std::cout << "[Gloss] WriteMemory: 0x" << std::hex << addr << " size=" << size << std::dec << std::endl;
    if (addr >= 0x1000000 && addr < 0xC0000000) {
        memcpy(g_guest_memory + addr, g_guest_memory + data_ptr, size);
    }
}

void load_and_boot_modded() {
    // //MODDED INSTANCE//
    std::cout << "[ModLoader] Initializing Swordigo RL Modded Instance..." << std::endl;
    
    std::string swordigo_path = get_data_path("libswordigo.so");
    std::string gloss_path = get_data_path("libGlossHook.so");
    std::string mini_path = get_data_path("libmini.so");
    
    // Use distinct memory regions for each module
    uint32_t addr_swordigo = 0x1000000;
    uint32_t addr_gloss    = 0x4000000; // Give plenty of space
    uint32_t addr_mini     = 0x5000000;
    
    // 1. Load All ELF Modules
    if (g_loader->load(&g_main_mod, swordigo_path, addr_swordigo) != 0) {
        std::cerr << "[ModLoader] Failed to load libswordigo.so" << std::endl;
        exit(1);
    }
    if (g_loader->load(&g_gloss_mod, gloss_path, addr_gloss) != 0) {
        std::cerr << "[ModLoader] Failed to load libGlossHook.so" << std::endl;
        exit(1);
    }
    if (g_loader->load(&g_mini_mod, mini_path, addr_mini) != 0) {
        std::cerr << "[ModLoader] Failed to load libmini.so" << std::endl;
        exit(1);
    }
    
    // 2. Internal Relocations
    g_loader->relocate(&g_main_mod);
    g_loader->relocate(&g_gloss_mod);
    g_loader->relocate(&g_mini_mod);
    
    // 3. Multi-Module Symbol Resolution
    // Dependencies: mini depends on gloss, gloss depends on bridge/standard libs.
    // swordigo is the base.
    g_loader->resolve_multi(&g_main_mod, &g_bridge, {}, GUEST_GLOBALS_BASE);
    g_loader->resolve_multi(&g_gloss_mod, &g_bridge, {&g_main_mod}, GUEST_GLOBALS_BASE);
    g_loader->resolve_multi(&g_mini_mod, &g_bridge, {&g_gloss_mod, &g_main_mod}, GUEST_GLOBALS_BASE);
    
    std::cout << "[ModLoader] All modules loaded and symbols cross-resolved." << std::endl;

    // Initialize Emulator
    g_emulator = new Emulator(g_guest_memory, GUEST_MEM_SIZE);
    g_emulator->set_bridge(&g_bridge);

    // Run dynamic initializers (.init_array) for all modules
    auto run_inits = [&](so_module* mod) {
        if (mod->init_array_vaddr != 0 && mod->init_array_size > 0) {
            int count = mod->init_array_size / 4;
            std::cout << "[ModLoader] Executing " << count << " initializers for " << mod->soname << "..." << std::endl;
            uint32_t* init_array = (uint32_t*)(g_guest_memory + mod->init_array_vaddr);
            for (int i = 0; i < count; i++) {
                if (init_array[i] != 0) g_emulator->call(init_array[i], {});
            }
        }
    };
    run_inits(&g_main_mod);
    run_inits(&g_gloss_mod);
    run_inits(&g_mini_mod);

    // Setup JNI structures
    uint32_t env_ptr = setup_jni_env(g_guest_memory);
    uint32_t vm_ptr = 0x11000;

    // Register GlossHook bridges
    g_bridge.register_handler("GlossInit", bridge_GlossInit);
    g_bridge.register_handler("GlossEnableLog", bridge_GlossEnableLog);
    g_bridge.register_handler("GlossHookAddr", bridge_GlossHookAddr);
    g_bridge.register_handler("WriteMemory", bridge_WriteMemory);

    // 4. Call SwMini Load Sequence
    uint32_t earlyLoad = g_loader->get_symbol_vaddr(&g_mini_mod, "earlyLoad");
    uint32_t midLoad = g_loader->get_symbol_vaddr(&g_mini_mod, "midLoad");
    uint32_t lateLoad = g_loader->get_symbol_vaddr(&g_mini_mod, "lateLoad");
    uint32_t mini_onload = g_loader->get_symbol_vaddr(&g_mini_mod, "JNI_OnLoad");

    if (mini_onload) {
        std::cout << "[ModLoader] Calling libmini.so JNI_OnLoad..." << std::endl;
        g_emulator->call(mini_onload, {vm_ptr, 0});
    }

    if (earlyLoad) {
        std::cout << "[ModLoader] Calling SwMini earlyLoad..." << std::endl;
        g_emulator->call(earlyLoad, {});
    }

    if (midLoad) {
        std::cout << "[ModLoader] Calling SwMini midLoad..." << std::endl;
        g_emulator->call(midLoad, {});
    }

    // 5. Vanilla Boot Sequence
    uint32_t setFilesDir = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_setFilesDir");
    uint32_t setCacheDir = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_setCacheDir");
    uint32_t setAssetMgr = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_setAssetManager");
    uint32_t handleAppLaunch = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_handleApplicationLaunch");
    uint32_t initMusicPlayer = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_MusicPlayer_initMusicPlayer");
    uint32_t setupNative = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_setupNativeInterface");
    uint32_t setupApp = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_setupApplication");
    uint32_t setApplicationViewSize = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_setApplicationViewSize");
    uint32_t applicationDidBecomeActive = g_loader->get_symbol_vaddr(&g_main_mod, "Java_com_touchfoo_swordigo_Native_applicationDidBecomeActive");

    uint32_t path_ptr = 0x20000;
    uint32_t files_dir = path_ptr;
    strcpy((char*)(g_guest_memory + files_dir), g_save_dir.c_str());
    path_ptr += 256;
    
    uint32_t cache_dir = path_ptr;
    strcpy((char*)(g_guest_memory + cache_dir), g_cache_dir.c_str());
    path_ptr += 256;

    if (setFilesDir) g_emulator->call(setFilesDir, {env_ptr, 0, files_dir});
    if (setCacheDir) g_emulator->call(setCacheDir, {env_ptr, 0, cache_dir});
    if (setAssetMgr) g_emulator->call(setAssetMgr, {env_ptr, 0, 0x55555555}); 
    if (handleAppLaunch) g_emulator->call(handleAppLaunch, {env_ptr, 0});
    if (initMusicPlayer) g_emulator->call(initMusicPlayer, {env_ptr, 0x22222222});
    if (setupNative) g_emulator->call(setupNative, {env_ptr, 0});
    if (setupApp) g_emulator->call(setupApp, {env_ptr, 0});
    if (setApplicationViewSize) g_emulator->call(setApplicationViewSize, {env_ptr, 0, 960, 544, 1});
    if (applicationDidBecomeActive) g_emulator->call(applicationDidBecomeActive, {env_ptr, 0});

    if (lateLoad) {
        std::cout << "[ModLoader] Calling SwMini lateLoad..." << std::endl;
        g_emulator->call(lateLoad, {});
    }

    std::cout << "[ModLoader] Boot sequence complete. Entering game loop." << std::endl;
    start_game_loop(env_ptr);
}
