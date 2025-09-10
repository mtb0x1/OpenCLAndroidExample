adb devices

adb shell ls -l /vendor/lib*/libOpenCL.so

adb pull /vendor/lib/libOpenCL.so OpenCLAndroidExample/app/src/main/cpp/OpenCL/libOpenCL32.so

adb pull /vendor/lib64/libOpenCL.so OpenCLAndroidExample/app/src/main/cpp/OpenCL/libOpenCL64.so


adb -s emulator-5554 push build_cli/openclandroidexample_cli /tmp/

adb shell

>$ logcat | grep -i OpenCL-EXAMPLE-logs
