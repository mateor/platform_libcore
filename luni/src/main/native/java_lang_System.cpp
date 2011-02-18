/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "System"

#include "JNIHelp.h"
#include "JniConstants.h"
#include "ScopedLocalRef.h"
#include "ScopedUtfChars.h"

#include "openssl/opensslv.h"
#include "zlib.h"

#include <string>
#include <vector>

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>

static jobjectArray toStringArray(JNIEnv* env, const std::vector<std::string>& strings) {
    jobjectArray result = env->NewObjectArray(strings.size(), JniConstants::stringClass, NULL);
    for (size_t i = 0; i < strings.size(); ++i) {
        ScopedLocalRef<jstring> s(env, env->NewStringUTF(strings[i].c_str()));
        env->SetObjectArrayElement(result, i, s.get());
    }
    return result;
}

static jstring System_getEnvByName(JNIEnv* env, jclass, jstring javaName) {
    ScopedUtfChars name(env, javaName);
    if (name.c_str() == NULL) {
        return NULL;
    }
    return env->NewStringUTF(getenv(name.c_str()));
}

static jstring System_getEnvByIndex(JNIEnv* env, jclass, jint index) {
    // Pointer to complete environment.
    extern char** environ;
    return env->NewStringUTF(environ[index]);
}

// Sets a field via JNI. Used for the standard streams, which are read-only otherwise.
static void System_setFieldImpl(JNIEnv* env, jclass clazz,
        jstring javaName, jstring javaSignature, jobject object) {
    ScopedUtfChars name(env, javaName);
    if (name.c_str() == NULL) {
        return;
    }
    ScopedUtfChars signature(env, javaSignature);
    if (signature.c_str() == NULL) {
        return;
    }
    jfieldID fieldID = env->GetStaticFieldID(clazz, name.c_str(), signature.c_str());
    env->SetStaticObjectField(clazz, fieldID, object);
}

static jobjectArray System_specialProperties(JNIEnv* env, jclass) {
    std::vector<std::string> properties;

    struct utsname info;
    uname(&info);
    properties.push_back(std::string("os.arch=") + info.machine);
    properties.push_back(std::string("os.name=") + info.sysname);
    properties.push_back(std::string("os.version=") + info.release);

    char path[PATH_MAX];
    properties.push_back(std::string("user.dir=") + getcwd(path, sizeof(path)));

    properties.push_back("android.zlib.version=" ZLIB_VERSION);
    properties.push_back("android.openssl.version=" OPENSSL_VERSION_TEXT);

    return toStringArray(env, properties);
}

static JNINativeMethod gMethods[] = {
    NATIVE_METHOD(System, getEnvByIndex, "(I)Ljava/lang/String;"),
    NATIVE_METHOD(System, getEnvByName, "(Ljava/lang/String;)Ljava/lang/String;"),
    NATIVE_METHOD(System, setFieldImpl, "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/Object;)V"),
    NATIVE_METHOD(System, specialProperties, "()[Ljava/lang/String;"),
};
int register_java_lang_System(JNIEnv* env) {
    return jniRegisterNativeMethods(env, "java/lang/System", gMethods, NELEM(gMethods));
}
