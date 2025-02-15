#ifndef CLICKER_HPP
#define CLICKER_HPP

#pragma comment(lib, "User32.lib")

#include "config.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <atomic>
#include <windows.h>

class Clicker
{
public:
    explicit Clicker(const Config &config)
        : config(config), running(true), paused(false), hHook(NULL), hookThreadId(0)
    {
        initKeyMap();
        // 初始化各个键的状态
        for (auto &pair : keyMap)
        {
            keyHeld[pair.second] = false;
            triggerHeld[pair.second] = false;
        }
        // 保存当前实例，供低级钩子回调使用
        instance = this;
    }

    // 启动时，新建线程安装低级键盘钩子，并运行消息循环
    void start()
    {
        std::cout << "[Info] Clicker started, installing keyboard hook..." << std::endl;
        hookThread = std::thread(&Clicker::runHookLoop, this);
    }

    // 停止时，发送退出消息给钩子线程，并等待线程退出
    void stop()
    {
        running = false;
        if (hookThread.joinable())
        {
            // 向钩子线程的消息队列发送退出消息
            PostThreadMessage(hookThreadId, WM_QUIT, 0, 0);
            hookThread.join();
        }
        std::cout << "[Info] Clicker stopped." << std::endl;
    }

private:
    Config config;
    std::atomic<bool> running;
    std::atomic<bool> paused;
    std::thread hookThread;
    DWORD hookThreadId; // 钩子线程的ID
    HHOOK hHook;        // 低级键盘钩子句柄

    // 键盘映射表：从字符串到虚拟键码
    std::unordered_map<std::string, int> keyMap;
    // 用于去抖动，记录各个键当前是否处于“已按下”状态
    std::unordered_map<int, bool> keyHeld;
    // 用于记录“hold”触发模式下，已模拟按下的触发键（防止重复发送 keydown）
    std::unordered_map<int, bool> triggerHeld;

    // 设定全局静态实例指针，供钩子回调使用（仅支持单实例）
    static Clicker *instance;

    // 初始化常用键的映射
    void initKeyMap()
    {
        keyMap = {
            {"LMB", VK_LBUTTON},
            {"RMB", VK_RBUTTON},
            {"MMB", VK_MBUTTON},
            {"SHIFT", VK_SHIFT},
            {"CTRL", VK_CONTROL},
            {"ALT", VK_MENU},
            {"ENTER", VK_RETURN},
            {"ESC", VK_ESCAPE},
            {"SPACE", VK_SPACE},
            {"TAB", VK_TAB},
            {"UP", VK_UP},
            {"DOWN", VK_DOWN},
            {"LEFT", VK_LEFT},
            {"RIGHT", VK_RIGHT},

            // 功能键 F1-F12
            {"F1", VK_F1}, {"F2", VK_F2}, {"F3", VK_F3}, {"F4", VK_F4},
            {"F5", VK_F5}, {"F6", VK_F6}, {"F7", VK_F7}, {"F8", VK_F8},
            {"F9", VK_F9}, {"F10", VK_F10}, {"F11", VK_F11}, {"F12", VK_F12},

            // 字母键 A-Z
            {"A", 0x41}, {"B", 0x42}, {"C", 0x43}, {"D", 0x44},
            {"E", 0x45}, {"F", 0x46}, {"G", 0x47}, {"H", 0x48},
            {"I", 0x49}, {"J", 0x4A}, {"K", 0x4B}, {"L", 0x4C},
            {"M", 0x4D}, {"N", 0x4E}, {"O", 0x4F}, {"P", 0x50},
            {"Q", 0x51}, {"R", 0x52}, {"S", 0x53}, {"T", 0x54},
            {"U", 0x55}, {"V", 0x56}, {"W", 0x57}, {"X", 0x58},
            {"Y", 0x59}, {"Z", 0x5A},

            // 数字键 0-9
            {"0", 0x30}, {"1", 0x31}, {"2", 0x32}, {"3", 0x33},
            {"4", 0x34}, {"5", 0x35}, {"6", 0x36}, {"7", 0x37},
            {"8", 0x38}, {"9", 0x39},

            // 小键盘数字键
            {"NUM0", VK_NUMPAD0}, {"NUM1", VK_NUMPAD1}, {"NUM2", VK_NUMPAD2},
            {"NUM3", VK_NUMPAD3}, {"NUM4", VK_NUMPAD4}, {"NUM5", VK_NUMPAD5},
            {"NUM6", VK_NUMPAD6}, {"NUM7", VK_NUMPAD7}, {"NUM8", VK_NUMPAD8},
            {"NUM9", VK_NUMPAD9},

            // 其他特殊键
            {"CAPSLOCK", VK_CAPITAL}, {"SCROLLLOCK", VK_SCROLL},
            {"PAUSE", VK_PAUSE}, {"INSERT", VK_INSERT},
            {"DELETE", VK_DELETE}, {"HOME", VK_HOME},
            {"END", VK_END}, {"PAGEUP", VK_PRIOR}, {"PAGEDOWN", VK_NEXT},
            {"BACKSPACE", VK_BACK}, {"PRINTSCREEN", VK_SNAPSHOT},
            {"APPS", VK_APPS}, {"LWIN", VK_LWIN}, {"RWIN", VK_RWIN}
        };
    }

    // 低级键盘钩子回调函数（静态方法）
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode == HC_ACTION && instance != nullptr)
        {
            KBDLLHOOKSTRUCT *pKbd = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);
            // 处理按下和松开事件
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)
            {
                instance->ProcessKeyDown(pKbd->vkCode);
            }
            else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP)
            {
                instance->ProcessKeyUp(pKbd->vkCode);
            }
        }
        return CallNextHookEx(instance ? instance->hHook : NULL, nCode, wParam, lParam);
    }

    // 钩子线程入口：安装钩子并运行消息循环
    void runHookLoop()
    {
        hookThreadId = GetCurrentThreadId();
        hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
        if (!hHook)
        {
            std::cerr << "[Error] Failed to install keyboard hook." << std::endl;
            return;
        }
        std::cout << "[Info] Keyboard hook installed." << std::endl;

        MSG msg;
        while (running && GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        UnhookWindowsHookEx(hHook);
        std::cout << "[Info] Keyboard hook uninstalled." << std::endl;
    }

    // 处理按键按下事件（防重复：若该键已处于按下状态，则不重复处理）
    void ProcessKeyDown(int vkCode)
    {
        if (keyHeld[vkCode])
            return; // 已处理过，忽略重复事件
        keyHeld[vkCode] = true;
        std::cout << "[Log] Key down: " << vkCode << std::endl;

        // 检查是否为暂停键
        int pauseKeyCode = keyMap[config.pauseKey];
        if (vkCode == pauseKeyCode)
        {
            paused = !paused;
            std::cout << "[Log] " << (paused ? "Paused" : "Resumed") << " via pause key." << std::endl;
            return;
        }
        if (paused)
            return; // 暂停状态下忽略其它按键

        // 检查每个监听器配置
        for (const auto &listener : config.listeners)
        {
            // 查找监听键在映射表中的虚拟键码
            if (keyMap.find(listener.listenKey) == keyMap.end())
            {
                std::cerr << "[Error] Listen key (" << listener.listenKey << ") not found in keyMap!" << std::endl;
                continue;
            }
            int listenKeyCode = keyMap[listener.listenKey];
            if (vkCode == listenKeyCode)
            {
                std::cout << "[Log] Listener key triggered: " << listener.listenKey << std::endl;
                // 对于该监听器，处理所有关联的触发器
                for (const auto &trigger : listener.triggers)
                {
                    if (keyMap.find(trigger.triggerKey) == keyMap.end())
                    {
                        std::cerr << "[Error] Trigger key (" << trigger.triggerKey << ") not found in keyMap!" << std::endl;
                        continue;
                    }
                    int triggerKeyCode = keyMap[trigger.triggerKey];
                    if (trigger.action == "click")
                    {
                        std::cout << "[Log] Triggering 'click' for key: " << trigger.triggerKey << std::endl;
                        simulateKeyPress(triggerKeyCode);
                    }
                    else if (trigger.action == "hold")
                    {
                        // 仅在首次按下时模拟按下，防止重复
                        if (!triggerHeld[triggerKeyCode])
                        {
                            std::cout << "[Log] Triggering 'hold' down for key: " << trigger.triggerKey << std::endl;
                            simulateKeyDown(triggerKeyCode);
                            triggerHeld[triggerKeyCode] = true;
                        }
                    }
                    else
                    {
                        std::cerr << "[Error] Unknown trigger action: " << trigger.action << std::endl;
                    }
                }
            }
        }
    }

    // 处理按键松开事件，针对“hold”动作释放已保持的触发键
    void ProcessKeyUp(int vkCode)
    {
        keyHeld[vkCode] = false;
        std::cout << "[Log] Key up: " << vkCode << std::endl;
        // 检查每个监听器中是否有对应“hold”动作需要释放
        for (const auto &listener : config.listeners)
        {
            if (keyMap.find(listener.listenKey) == keyMap.end())
                continue;
            int listenKeyCode = keyMap[listener.listenKey];
            if (vkCode == listenKeyCode)
            {
                for (const auto &trigger : listener.triggers)
                {
                    if (trigger.action == "hold")
                    {
                        if (keyMap.find(trigger.triggerKey) == keyMap.end())
                            continue;
                        int triggerKeyCode = keyMap[trigger.triggerKey];
                        if (triggerHeld[triggerKeyCode])
                        {
                            std::cout << "[Log] Releasing hold for key: " << trigger.triggerKey << std::endl;
                            simulateKeyUp(triggerKeyCode);
                            triggerHeld[triggerKeyCode] = false;
                        }
                    }
                }
            }
        }
    }

#ifdef _WIN32
    // 使用 SendInput API 进行按键模拟
    void simulateKeyPress(int key)
    {
        std::cout << "[Log] Simulating key press for key code: " << key << std::endl;
        simulateKeyDown(key);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        simulateKeyUp(key);
    }

    void simulateKeyDown(int key)
    {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        UINT sent = SendInput(1, &input, sizeof(INPUT));
        if (sent != 1)
        {
            std::cerr << "[Error] simulateKeyDown: Failed to send key down for key code " << key << std::endl;
        }
        else
        {
            std::cout << "[Log] simulateKeyDown: Key code " << key << " sent (" << sent << " event)" << std::endl;
        }
    }

    void simulateKeyUp(int key)
    {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = key;
        input.ki.dwFlags = KEYEVENTF_KEYUP;
        UINT sent = SendInput(1, &input, sizeof(INPUT));
        if (sent != 1)
        {
            std::cerr << "[Error] simulateKeyUp: Failed to send key up for key code " << key << std::endl;
        }
        else
        {
            std::cout << "[Log] simulateKeyUp: Key code " << key << " released (" << sent << " event)" << std::endl;
        }
    }
#endif
};

// 初始化静态实例指针
Clicker* Clicker::instance = nullptr;

#endif // CLICKER_HPP
