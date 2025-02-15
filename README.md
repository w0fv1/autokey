### AutoKey Clicker

# **AutoKey Clicker**
**AutoKey Clicker** 是一款 Windows 自动按键工具，支持监听特定按键并触发自定义按键操作。适用于游戏、办公自动化等场景，可通过 `config.json` 配置实现灵活的按键映射。

---

## **功能特性**
- **按键监听**：支持监听任意键盘按键并触发自动输入。
- **自定义触发**：支持 `click` 和 `hold` 模式：
  - **click** - 触发一次按键点击。
  - **hold** - 持续按住按键，直到监听键释放。
- **快捷键启停**：支持 `pauseKey`，用户可随时暂停或恢复自动按键功能。
- **低级键盘钩子**：使用 `SetWindowsHookEx(WH_KEYBOARD_LL, …)` 捕获键盘输入，兼容全屏模式的游戏。
- **SendInput API**：模拟键盘输入，提高游戏兼容性。
- **详细日志输出**：提供按键状态日志，便于调试和优化。

---

## **安装与编译**
### **编译环境**
- **操作系统**：Windows 7/10/11
- **编译工具**：Microsoft Visual Studio 2022（或更高版本）
- **依赖库**：
  - Windows API (`User32.lib`)
  - `nlohmann/json`（用于解析 `config.json`）

### **编译命令**
在 **PowerShell** 终端执行：
```powershell
PS D:\project\autokey> cmd /k '"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat"'
PS D:\project\autokey> del main.exe & del main.obj & cl main.cpp /EHsc /Fe:main.exe /link User32.lib & main.exe
```

---

## **使用说明**
### **1. 运行程序**
编译完成后，在终端运行：
```powershell
PS D:\project\autokey> .\main.exe
```
程序会自动加载 `config.json` 并监听按键。

### **2. 配置文件 (`config.json`)**
配置文件 `config.json` 控制按键监听规则：
```json
{
  "pauseKey": "F11",
  "listeners": [
      {
          "listenKey": "A",
          "triggers": [
              {
                  "triggerKey": "S",
                  "action": "hold"
              }
          ]
      }
  ]
}
```
#### **配置项说明**
| 配置项 | 说明 |
|--------|------|
| `pauseKey` | 快捷键启停功能的按键（按 `F11` 触发暂停或恢复） |
| `listenKey` | 监听按键，按下后触发 `triggers` 动作 |
| `triggerKey` | 被触发的按键 |
| `action` | `"click"` 触发一次点击，`"hold"` 持续按下直到 `listenKey` 释放 |

### **3. 退出程序**
在终端按 `Enter` 退出：
```powershell
Press Enter to exit...
```

---

## **可能的用法**
### **1. 游戏按键增强**
示例：按 `A` 时自动按住 `S`，适用于游戏中的移动控制：
```json
{
  "listeners": [
      {
          "listenKey": "A",
          "triggers": [
              {
                  "triggerKey": "S",
                  "action": "hold"
              }
          ]
      }
  ]
}
```

### **2. 办公自动化**
示例：按 `CTRL+E` 自动输入 `ENTER`（适用于表单提交）：
```json
{
  "listeners": [
      {
          "listenKey": "CTRL",
          "triggers": [
              {
                  "triggerKey": "ENTER",
                  "action": "click"
              }
          ]
      }
  ]
}
```

---

## **局限性**
1. **部分游戏无法使用**
   - 某些使用 DirectInput 或 EAC（Easy Anti-Cheat）防作弊的游戏可能会屏蔽 `SendInput`。
   - 可能需要更底层的驱动级输入模拟。

2. **管理员权限**
   - 如果目标游戏或应用以 **管理员权限** 运行，本程序可能需要 **以管理员权限运行** 才能生效。

3. **键盘钩子可能被拦截**
   - 某些应用程序可能会阻止全局钩子 (`WH_KEYBOARD_LL`)，导致按键监听失效。

4. **延迟影响**
   - `SendInput` 在某些情况下可能会被系统任务调度延迟，影响响应速度。

---

## **版权与免责声明**
- 本软件仅用于**个人学习和办公自动化**，请勿用于任何违反游戏规则或其他不当用途。
- 开发者不对任何可能的使用后果负责。
