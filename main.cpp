#include "config.hpp"
#include "clicker.hpp"

int main()
{
    Config config("config.json"); // 读取或创建配置文件
    config.print();               // 打印配置

    Clicker clicker(config);
    clicker.start();

    std::cout << "Press Enter to exit...\n";
    std::cin.get(); // 等待用户输入退出

    clicker.stop(); // 停止监听
    return 0;
}
// cmd /k '"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat"'
// del main.exe & del *.obj & cl main.cpp clicker.cpp config.cpp /EHsc /Fe:main.exe /link User32.lib & main.exe
