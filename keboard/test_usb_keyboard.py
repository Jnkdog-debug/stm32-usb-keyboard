#!/usr/bin/env python3
"""
USB Keyboard Test Tool
用于测试 STM32F407 USB 键盘的工具脚本

使用方法:
  python3 test_usb_keyboard.py

要求:
  - Python 3.6+
  - 可选: pynput (用于键盘输入监听)
"""

import os
import sys
import time
import platform

def print_header(title):
    """打印美观的标题"""
    print("\n" + "="*60)
    print(f"  {title}")
    print("="*60)

def test_basic_functionality():
    """基础功能测试"""
    print_header("Basic Functionality Test")
    
    print("\n✓ 准备步骤:")
    print("  1. 将 STM32F407 USB 键盘连接到 PC")
    print("  2. 打开任意文本编辑器 (记事本/gedit)")
    print("  3. 点击文本框获得焦点")
    print("  4. 按下键盘上的按键")
    
    input("\n按 Enter 键开始测试...")
    
    test_cases = [
        ("按键 0", "输入: 1"),
        ("按键 1", "输入: 2"),
        ("按键 2", "输入: 3"),
        ("按键 3", "输入: 4"),
        ("按键 4", "输入: 5"),
        ("按键 5", "输入: 6"),
        ("按键 6", "输入: 7"),
        ("按键 7", "输入: 8"),
        ("按键 8", "输入: 9"),
    ]
    
    passed = 0
    failed = 0
    
    for key, expected in test_cases:
        result = input(f"\n{key} -> {expected} [y/n/skip]: ").lower().strip()
        if result == 'y':
            passed += 1
            print("  ✓ 通过")
        elif result == 'n':
            failed += 1
            print("  ✗ 失败")
        else:
            print("  ⊘ 跳过")
    
    print(f"\n结果: {passed} 通过, {failed} 失败")
    return failed == 0

def test_multikey_press():
    """多键同时按下测试"""
    print_header("Multi-Key Press Test")
    
    print("\n尝试同时按下多个按键 (最多6个)")
    print("例如: 同时按下按键 0, 4, 8")
    print("预期输出: 159 (在任意顺序下都应该输出这三个数字)")
    
    input("\n按 Enter 键开始多键测试...")
    
    result = input("\n看到同时输出的数字了吗? [y/n]: ").lower().strip()
    
    return result == 'y'

def test_rapid_press():
    """快速连续按键测试"""
    print_header("Rapid Key Press Test")
    
    print("\n快速连续按下多个不同的按键")
    print("预期: 所有按键都应该被正确识别,无遗漏")
    
    input("\n按 Enter 键开始快速按键测试...")
    
    result = input("\n所有按键都被识别了吗? [y/n]: ").lower().strip()
    
    return result == 'y'

def test_debounce():
    """防抖测试"""
    print_header("Debounce Test")
    
    print("\n长按一个按键 (2-3秒不松开)")
    print("预期: 按键应该只输入一次,不会重复输入")
    
    input("\n按 Enter 键开始防抖测试...")
    
    result = input("\n按键只输入了一次吗? [y/n]: ").lower().strip()
    
    return result == 'y'

def test_device_detection():
    """USB设备检测"""
    print_header("USB Device Detection")
    
    system = platform.system()
    
    if system == "Linux":
        print("\n检查 USB 设备...")
        os.system("echo '\\n--- lsusb output ---'")
        os.system("lsusb")
        
        print("\n\n--- dmesg output (recent) ---")
        os.system("dmesg | tail -20")
        
        print("\n\n预期: 应该看到 STMicroelectronics STM32F4xx 设备")
        
    elif system == "Windows":
        print("\n请按以下步骤检查设备:")
        print("  1. 打开 设备管理器 (Win+X -> 设备管理器)")
        print("  2. 展开 人机接口设备 (Human Interface Devices)")
        print("  3. 查找 STM32F407 USB Keyboard 或类似设备")
        print("  4. 确认设备状态正常 (无黄色警告标记)")
        
    elif system == "Darwin":
        print("\n检查 USB 设备...")
        os.system("system_profiler SPUSBDataType | grep -A 10 'STM'")
    
    result = input("\nUSB 设备被正确识别了吗? [y/n]: ").lower().strip()
    
    return result == 'y'

def show_test_summary(results):
    """显示测试总结"""
    print_header("Test Summary")
    
    tests = [
        ("USB Device Detection", results.get('detection', False)),
        ("Basic Functionality", results.get('basic', False)),
        ("Multi-Key Press", results.get('multikey', False)),
        ("Rapid Key Press", results.get('rapid', False)),
        ("Debounce", results.get('debounce', False)),
    ]
    
    passed = sum(1 for _, result in tests if result)
    total = len(tests)
    
    print()
    for name, result in tests:
        status = "✓ 通过" if result else "✗ 失败/跳过"
        print(f"  {name}: {status}")
    
    print(f"\n总体结果: {passed}/{total} 通过")
    
    if passed == total:
        print("\n🎉 所有测试通过! USB键盘工作正常!")
        return True
    else:
        print("\n⚠️  部分测试失败,请检查硬件连接和驱动")
        return False

def show_troubleshooting():
    """显示故障排查建议"""
    print_header("Troubleshooting Tips")
    
    print("""
如果测试失败,请按以下步骤排查:

1. USB 无法识别
   - 检查 USB 线连接
   - 尝试不同的 USB 端口
   - 检查设备管理器中是否有未知设备

2. 部分按键无法输入
   - 检查矩阵键盘硬件连接
   - 用万用表测量 GPIO 引脚电压
   - 查看 UART 调试输出确认是否检测到按键

3. 输入重复或错误
   - 增加消抖时间 (DEBOUNCE_TIME)
   - 检查按键硬件质量
   - 检查矩阵扫描代码

4. 需要更多帮助
   - 查看 USB_KEYBOARD_VERIFICATION.txt
   - 查看 README.md
   - 查看 UART 串口输出进行调试

更新固件步骤:
  make clean
  make
  sudo st-flash write build/keboard.bin 0x8000000
    """)

def main():
    """主函数"""
    print("\n" + "="*60)
    print("  STM32F407 USB Keyboard Test Tool")
    print("="*60)
    
    print("\n这个工具将帮助你验证 USB 键盘的功能")
    print("按照提示完成各个测试步骤")
    
    # 确认准备好
    ready = input("\n你已经准备好进行测试了吗? (y/n): ").lower().strip()
    if ready != 'y':
        print("请准备好后重新运行")
        return
    
    results = {}
    
    # 测试1: USB设备检测
    results['detection'] = test_device_detection()
    
    if results['detection']:
        # 如果检测到USB设备,进行其他测试
        results['basic'] = test_basic_functionality()
        results['multikey'] = test_multikey_press()
        results['rapid'] = test_rapid_press()
        results['debounce'] = test_debounce()
    else:
        print("\n⚠️  USB 设备未被识别,跳过其他测试")
        results['basic'] = False
        results['multikey'] = False
        results['rapid'] = False
        results['debounce'] = False
    
    # 显示总结
    success = show_test_summary(results)
    
    # 显示故障排查提示
    if not success:
        show_troubleshooting()
    
    print("\n" + "="*60)
    print("  测试完成")
    print("="*60 + "\n")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n测试被中断")
        sys.exit(1)
