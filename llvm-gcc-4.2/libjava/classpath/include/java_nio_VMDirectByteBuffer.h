/* DO NOT EDIT THIS FILE - it is machine generated */

#ifndef __java_nio_VMDirectByteBuffer__
#define __java_nio_VMDirectByteBuffer__

#include <jni.h>

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT jobject JNICALL Java_java_nio_VMDirectByteBuffer_allocate (JNIEnv *env, jclass, jint);
JNIEXPORT void JNICALL Java_java_nio_VMDirectByteBuffer_free (JNIEnv *env, jclass, jobject);
JNIEXPORT jbyte JNICALL Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I (JNIEnv *env, jclass, jobject, jint);
JNIEXPORT void JNICALL Java_java_nio_VMDirectByteBuffer_get__Lgnu_classpath_Pointer_2I_3BII (JNIEnv *env, jclass, jobject, jint, jbyteArray, jint, jint);
JNIEXPORT void JNICALL Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2IB (JNIEnv *env, jclass, jobject, jint, jbyte);
JNIEXPORT void JNICALL Java_java_nio_VMDirectByteBuffer_put__Lgnu_classpath_Pointer_2I_3BII (JNIEnv *env, jclass, jobject, jint, jbyteArray, jint, jint);
JNIEXPORT jobject JNICALL Java_java_nio_VMDirectByteBuffer_adjustAddress (JNIEnv *env, jclass, jobject, jint);
JNIEXPORT void JNICALL Java_java_nio_VMDirectByteBuffer_shiftDown (JNIEnv *env, jclass, jobject, jint, jint, jint);

#ifdef __cplusplus
}
#endif

#endif /* __java_nio_VMDirectByteBuffer__ */
