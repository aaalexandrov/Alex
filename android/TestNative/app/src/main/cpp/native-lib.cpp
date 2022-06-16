#include <jni.h>
#include <string>
#include <math.h>
#include "SyncFloat.h"

void Test()
{
    float one = 1;
    float zero = 0;
    float o = one + zero;
    float two = o + one;
    float four = two * two;
    float minFour = -four;
    float minTwo = minFour / two;
    float s = sinf(minTwo);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_testnative_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */)
{
    Test();
    SyncFloat::Test();
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
