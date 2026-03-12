#include "qwen3_asr.h"

#include <unordered_map>
#include <jni.h>

int available_ctx_id = 0;
std::unordered_map<int, qwen3_asr::Qwen3ASR> ctx_map;
std::unordered_map<int, jobject> callback_map;
JavaVM *g_jvm = nullptr;

int new_ctx_id() {
    return available_ctx_id++;
}


#ifdef __cplusplus
extern "C" {
#endif

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
   g_jvm = vm;
    return JNI_VERSION_1_6;
}

/**
 * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
 * Method:    setProgressCallback
 * Signature: (Ljava/util/function/BiConsumer;)V
 */
JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_setProgressCallback(JNIEnv *env, jobject thiz, jobject callback) {
    if (const jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I")); ctx_id != -1) {
        if (callback_map.contains(ctx_id)) {
            env->DeleteGlobalRef(callback_map[ctx_id]);
        }
        // Create global reference to callback object
        callback_map[ctx_id] = env->NewGlobalRef(callback);
        ctx_map[ctx_id].set_progress_callback([ctx_id](const int tokens_generated, const int max_tokens) {
            if (!callback_map.contains(ctx_id)) {
                return;
            }
            jobject global_callback = callback_map[ctx_id];
            if (!global_callback || !g_jvm) {
                return;
            }
            
            // Get JNI environment for current thread
            JNIEnv *env = nullptr;
            jint result = g_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
            bool attached = false;
            if (result == JNI_EDETACHED) {
                result = g_jvm->AttachCurrentThread(reinterpret_cast<void **>(&env), nullptr);
                if (result != JNI_OK || !env) {
                    return;
                }
                attached = true;
            } else if (result != JNI_OK || !env) {
                return;
            }
            
            // Call Java method
            jclass cls = env->GetObjectClass(global_callback);
            if (cls) {
                jmethodID methodID = env->GetMethodID(cls, "accept", "(II)V");
                if (methodID) {
                    env->CallVoidMethod(global_callback, methodID, tokens_generated, max_tokens);
                    env->ExceptionClear();
                }
            }

            // Detach current thread only if we attached it
            if (attached) {
                g_jvm->DetachCurrentThread();
            }
        });
    }
}

/**
 * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
 * Method:    isLoaded
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_isLoaded(JNIEnv *env, jobject thiz) {
    const jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
    return ctx_map.contains(ctx_id);
}


/**
 * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
 * Method:    load
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_load(JNIEnv *env, jobject thiz, jstring modelPath) {
    // Get model path from Java string
    const char *model_path = env->GetStringUTFChars(modelPath, nullptr);
    if (model_path == nullptr) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "Model path is null");
        return;
    }

    // Create new Qwen3ASR instance
    int ctx_id = new_ctx_id();
    ctx_map[ctx_id] = qwen3_asr::Qwen3ASR();

    // Load model
    const bool success = ctx_map[ctx_id].load_model(model_path);
    // Release Java string
    env->ReleaseStringUTFChars(modelPath, model_path);

    if (!success) {
        env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), ("Model load failed: " + ctx_map[ctx_id].get_error()).c_str());
        return;
    }

    env->SetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"), ctx_id);
    fprintf(stderr, "Model load state: %d\n", success);
}

/**
 * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
 * Method:    free
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_free(JNIEnv *env, jobject thiz) {
    jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
    if (ctx_map.contains(ctx_id)) {
        env->DeleteGlobalRef(callback_map[ctx_id]);
        ctx_map.erase(ctx_id);
        callback_map.erase(ctx_id);
    }
}

/**
 * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
 * Method:    transcribe
 * Signature: ([FIio/github/jaffe2718/qwen3asr4j/TranscribeParams;)Lio/github/jaffe2718/qwen3asr4j/TranscribeResult;
 */
JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_transcribe(JNIEnv *env, jobject thiz, jfloatArray samples, jint nSamples, jobject params) {
    jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
    if (!ctx_map.contains(ctx_id)) {
        env->ThrowNew(env->FindClass("java/lang/IllegalStateException"), "Context not loaded");
        return nullptr;
    }
    // get model context (reference, not copy)
    qwen3_asr::Qwen3ASR &ctx = ctx_map[ctx_id];
    // get audio samples
    jfloat *samples_ptr = env->GetFloatArrayElements(samples, nullptr);
    const auto lang = (jstring)env->GetObjectField(params, env->GetFieldID(env->GetObjectClass(params), "language", "Ljava/lang/String;"));
    const char *lang_str = env->GetStringUTFChars(lang, nullptr);
    // convert java array to c++ array
    qwen3_asr::transcribe_params t_params {
        env->GetIntField(params, env->GetFieldID(env->GetObjectClass(params), "maxTokens", "I")),
        lang_str,
        env->GetIntField(params, env->GetFieldID(env->GetObjectClass(params), "nThreads", "I")),
        (bool)env->GetBooleanField(params, env->GetFieldID(env->GetObjectClass(params), "printProgress", "Z")),
        (bool)env->GetBooleanField(params, env->GetFieldID(env->GetObjectClass(params), "printTiming", "Z"))
    };

    qwen3_asr::transcribe_result res = ctx.transcribe(samples_ptr, nSamples, t_params);

    // create TranscribeResult object
    jclass res_cls = env->FindClass("io/github/jaffe2718/qwen3asr4j/TranscribeResult");
    jmethodID res_ctor = env->GetMethodID(res_cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;ZLjava/lang/String;JJJJJ)V");
    jobject res_obj = env->NewObject(res_cls, res_ctor, env->NewStringUTF(res.language.c_str()), env->NewStringUTF(res.text.c_str()), res.success, env->NewStringUTF(res.error_msg.c_str()), res.t_load_ms, res.t_mel_ms, res.t_encode_ms, res.t_decode_ms, res.t_total_ms);

    // release resources
    env->ReleaseFloatArrayElements(samples, samples_ptr, 0);
    env->ReleaseStringUTFChars(lang, lang_str);

    return res_obj;
}

#ifdef __cplusplus
}
#endif