#pragma once

#include "overlay.h"

// namespace overlay {
void overlay::Update() {
  if (!globals::show_menu)
    return;

  static ImGuiWindowFlags classFinderWindowFlags =
      ImGuiWindowFlags_AlwaysAutoResize;

  ImGui::Begin("HSR-GC", 0, classFinderWindowFlags);

  ImGui::BeginTabBar("##tabs");

  if (ImGui::BeginTabItem(u8"大世界选项")) // World
  {
    ImGui::Checkbox(u8"速度调整",
                    &globals::world::speed_hack); // Speed Modifier

    if (globals::world::speed_hack) {
      // Global
      ImGui::SliderFloat(u8"全局", &globals::world::global_speed, 0.1f,
                         10.f, "%.1f");
      // Dialogue
      ImGui::SliderFloat(u8"对话选项", &globals::world::dialogue_speed, 0.1f,
                         10.f, "%.1f");
    }

    ImGui::Checkbox(u8"反虚化",
                    &globals::world::peeking); // Peeking

    ImGui::Checkbox(u8"自动对话",
                    &globals::world::auto_dialogue); // Auto-Dialogue

    if (globals::world::auto_dialogue) {
      ImGui::Text(
          u8"也适用于热键（CAPSLOCK）"); // "also works on hotkey (CAPSLOCK)
      ImGui::Checkbox(u8"鼠标模式",
                      &globals::world::mouse_mode); // Mouse Mode
    }

    ImGui::Checkbox(u8"隐身",
                    &globals::world::invisibility); // Invisibility

    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(u8"战斗选项")) // Battle
  {
    ImGui::Checkbox(u8"速度调整",
                    &globals::battle::speed_hack); // Speed Modifier

    if (globals::battle::speed_hack) {

      // Battle
      ImGui::SliderFloat(u8"战斗选项", &globals::battle::battle_speed, 0.1f,
                         100.f, "%.1f");
    }

    // ImGui::Checkbox("Auto-Battle Unlock",
    // &globals::battle::auto_battle_unlock);

    ImGui::Checkbox(u8"强制自动战斗",
                    &globals::battle::force_battle); // Force Auto-Battle

    if (globals::battle::force_battle) {
      ImGui::Text(
          u8"如果你在战斗中启用了它，那么你需要做一些动作来使它起作用"); // "if
                                                                         // you
                                                                         // enabled
                                                                         // it
                                                                         // in
                                                                         // battle
                                                                         // then
                                                                         // you
                                     // need to do some action to make it work
    }

    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(u8"其他")) // Other
  {
    ImGui::Checkbox(u8"解锁帧数上限",
                    &globals::other::fps_unlock); // FPS Unlock

    if (globals::other::fps_unlock) {
      ImGui::InputInt("FPS", &globals::other::fps);
    }

    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(u8"调试")) // Debug
  {

    ImGui::Text(u8"阶段: %s", hooks::game::get_phase_in_text());
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(u8"设置")) // Settings
  {
    static bool pinWindow = false;

    if (ImGui::Checkbox(u8"窗口固定", &pinWindow)) // Pin Window
    {
      if (pinWindow) {
        classFinderWindowFlags |= ImGuiWindowFlags_NoMove;
        classFinderWindowFlags |= ImGuiWindowFlags_NoResize;
      } else {
        classFinderWindowFlags &= ~ImGuiWindowFlags_NoMove;
        classFinderWindowFlags &= ~ImGuiWindowFlags_NoResize;
      }
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"关闭")) // Unload - (translated to "closure")
      globals::unload = true;

    if (ImGui::Button(u8"保存设置")) { // Save Settings
      config::Save();
    }

    ImGui::EndTabItem();
  }

  if (ImGui::BeginTabItem(u8"关于")) // About
  {
    ImGui::Text("About this project");
    ImGui::Text("This project was created by Z4ee");
    ImGui::Text("This project is free, not for sell");
    ImGui::Text("List of Contributing Developers");
    ImGui::Text("Z4ee");
    ImGui::Text("ky-ler");

    ImGui::EndTabItem();
  }

  ImGui::EndTabBar();
  ImGui::End();
}

void overlay::Main() {
  uint64_t game_assembly = 0, unity_player = 0;

  while (!game_assembly && !unity_player) {
    game_assembly =
        reinterpret_cast<uint64_t>(GetModuleHandleA("gameassembly.dll"));
    unity_player =
        reinterpret_cast<uint64_t>(GetModuleHandleA("unityplayer.dll"));
  }

  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)features::dialogue::AutoDialogue,
               0, 0, 0);

  while (true) {
    features::speedhack::UpdateWorld(game_assembly, unity_player);
    features::speedhack::UpdateBattle(game_assembly, unity_player);
    features::other::Update(unity_player);

    HWND target_window = FindWindowA("UnityWndClass", nullptr);

    if (GetForegroundWindow() == target_window &&
        GetAsyncKeyState(globals::hotkeys::world_speed) & 1) {
      globals::world::speed_hack = !globals::world::speed_hack;
    } else if (GetForegroundWindow() == target_window &&
               GetAsyncKeyState(globals::hotkeys::battle_speed) & 1) {
      globals::battle::speed_hack = !globals::battle::speed_hack;
    } else if (GetForegroundWindow() == target_window &&
               GetAsyncKeyState(globals::hotkeys::auto_dialogue) & 1) {
      globals::world::auto_dialogue = !globals::world::auto_dialogue;
    } else if (GetForegroundWindow() == target_window &&
               GetAsyncKeyState(globals::hotkeys::invisibility) & 1) {
      globals::world::invisibility = !globals::world::invisibility;
    } else if (GetForegroundWindow() == target_window &&
               GetAsyncKeyState(globals::hotkeys::force_auto_battle) & 1) {
      globals::battle::force_battle = !globals::battle::force_battle;
    } else if (GetForegroundWindow() == target_window &&
               GetAsyncKeyState(globals::hotkeys::peeking) & 1) {
      globals::world::peeking = !globals::world::peeking;
    }

    Sleep(500);
  }
}

void overlay::Setup() {
  hooks::Setup();

  auto hook = ImGuiContextHook();
  hook.Callback = (ImGuiContextHookCallback)Update;
  hook.Type = ImGuiContextHookType_NewFramePost;
  ImGui::AddContextHook(ImGui::GetCurrentContext(), &hook);
}
//} // namespace overlay