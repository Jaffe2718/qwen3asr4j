#include "qwen3_asr.h"
#include "forced_aligner.h"

#include <map>
#include <jni.h>

int available_qwen3_asr_id = 0;
int available_forced_aligner_id = 0;
std::map<int, qwen3_asr::Qwen3ASR> qwen3_asr_map;
std::map<int, qwen3_asr::ForcedAligner> forced_aligner_map;
std::map<int, jobject> callback_map;
JavaVM *g_jvm = nullptr;

int new_qwen3_asr_id() {
    return available_qwen3_asr_id++;
}

int new_forced_aligner_id() {
    return available_forced_aligner_id++;
}


#ifdef __cplusplus
extern "C" {
#endif

    jint JNI_OnLoad(JavaVM *vm, void *reserved) {
       g_jvm = vm;
        return JNI_VERSION_1_6;
    }

    void slf4j_info(JNIEnv *env, jobject ggufModelWrapper, const char *msg) {
        jclass cls = env->GetObjectClass(ggufModelWrapper);
        jmethodID mid = env->GetMethodID(cls, "info", "(Ljava/lang/String;)V");
        env->CallVoidMethod(ggufModelWrapper, mid, env->NewStringUTF(msg));
    }


/*===================================io.github.jaffe2718.qwen3asr4j.Qwen3ASR===================================*/

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
            qwen3_asr_map[ctx_id].set_progress_callback([ctx_id](const int tokens_generated, const int max_tokens) {
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
                if (jclass cls = env->GetObjectClass(global_callback)) {
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
        return qwen3_asr_map.contains(ctx_id) && qwen3_asr_map[ctx_id].is_loaded();
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
     * Method:    getError
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_getError(JNIEnv *env, jobject thiz) {
        const jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (qwen3_asr_map.contains(ctx_id)) {
            return env->NewStringUTF(qwen3_asr_map[ctx_id].get_error().c_str());
        }
        return env->NewStringUTF("");
    }


    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
     * Method:    getConfig
     * Signature: ()Ljava/util/Map;
     */
    JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_getConfig(JNIEnv *env, jobject thiz) {
        const jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (qwen3_asr_map.contains(ctx_id)) {
            const qwen3_asr::text_decoder_config &config = qwen3_asr_map[ctx_id].get_config();
            // Create a new HashMap
            jclass hash_map_class = env->FindClass("java/util/HashMap");
            jmethodID hash_map_init = env->GetMethodID(hash_map_class, "<init>", "()V");
            jobject hash_map = env->NewObject(hash_map_class, hash_map_init);

            // Add config entries to the map
            jmethodID hash_map_put = env->GetMethodID(hash_map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("vocab_size"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.vocab_size));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("hidden_size"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.hidden_size));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("n_decoder_layers"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.n_decoder_layers));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("n_attention_heads"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.n_attention_heads));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("n_key_value_heads"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.n_key_value_heads));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("intermediate_size"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.intermediate_size));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("head_dim"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.head_dim));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("rms_norm_eps"), env->NewObject(env->FindClass("java/lang/Float"), env->GetMethodID(env->FindClass("java/lang/Float"), "<init>", "(F)V"), config.rms_norm_eps));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("rope_theta"), env->NewObject(env->FindClass("java/lang/Float"), env->GetMethodID(env->FindClass("java/lang/Float"), "<init>", "(F)V"), config.rope_theta));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("pad_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.pad_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("eos_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.eos_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_start_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.audio_start_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_end_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.audio_end_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_pad_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), config.audio_pad_token_id));

            return hash_map;
        }
        return nullptr;
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
     * Method:    load
     * Signature: (Ljava/lang/String;)I
     */
    JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_load(JNIEnv *env, jobject thiz, jstring modelPath) {
        // Get model path from Java string
        const char* model_path = env->GetStringUTFChars(modelPath, nullptr);
        if (model_path == nullptr) {
            env->ThrowNew(env->FindClass("java/io/FileNotFoundException"), "Qwen3ASR model path is null");
            return;
        }

        // Create new Qwen3ASR instance
        int ctx_id = new_qwen3_asr_id();
        qwen3_asr_map[ctx_id] = qwen3_asr::Qwen3ASR();

        // Load model
        const bool success = qwen3_asr_map[ctx_id].load_model(model_path);
        slf4j_info(env, thiz, ("Qwen3ASR<" + std::to_string(ctx_id) + "> model load state: " + std::string(success ? "success" : "failed")).c_str());

        // Release Java string
        env->ReleaseStringUTFChars(modelPath, model_path);

        if (!success) {
            env->ThrowNew(env->FindClass("java/io/FileNotFoundException"), ("Qwen3ASR model load failed: " + qwen3_asr_map[ctx_id].get_error()).c_str());
            qwen3_asr_map.erase(ctx_id);
            return;
        }

        env->SetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"), ctx_id);
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
     * Method:    free
     * Signature: ()V
     */
    JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_free(JNIEnv *env, jobject thiz) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (qwen3_asr_map.contains(ctx_id)) {
            env->DeleteGlobalRef(callback_map[ctx_id]);
            qwen3_asr_map.erase(ctx_id);
            callback_map.erase(ctx_id);
        }
        slf4j_info(env, thiz, ("Qwen3ASR<" + std::to_string(ctx_id) + "> context freed").c_str());
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
     * Method:    transcribe
     * Signature: ([FIio/github/jaffe2718/qwen3asr4j/TranscribeParams;)Lio/github/jaffe2718/qwen3asr4j/TranscribeResult;
     */
    JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_transcribe(JNIEnv *env, jobject thiz, jfloatArray samples, jint nSamples, jobject params) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (!qwen3_asr_map.contains(ctx_id)) {
            env->ThrowNew(env->FindClass("java/lang/NullPointerException"), ("Qwen3ASR<" + std::to_string(ctx_id) + "> context not loaded").c_str());
            return nullptr;
        }
        // get model context (reference, not copy)
        qwen3_asr::Qwen3ASR &ctx = qwen3_asr_map[ctx_id];
        // get audio samples
        jfloat *samples_ptr = env->GetFloatArrayElements(samples, nullptr);
        const auto lang = (jstring) env->GetObjectField(params, env->GetFieldID(env->GetObjectClass(params), "language", "Ljava/lang/String;"));
        const char * lang_str = env->GetStringUTFChars(lang, nullptr);
        // create transcribe_params
        qwen3_asr::transcribe_params t_params {
            env->GetIntField(params, env->GetFieldID(env->GetObjectClass(params), "maxTokens", "I")),
            lang_str,
            env->GetIntField(params, env->GetFieldID(env->GetObjectClass(params), "nThreads", "I")),
            (bool) env->GetBooleanField(params, env->GetFieldID(env->GetObjectClass(params), "printProgress", "Z")),
            (bool) env->GetBooleanField(params, env->GetFieldID(env->GetObjectClass(params), "printTiming", "Z"))
        };

        qwen3_asr::transcribe_result res = ctx.transcribe(samples_ptr, nSamples, t_params);

        // create TranscribeResult object
        jclass res_cls = env->FindClass("io/github/jaffe2718/qwen3asr4j/result/TranscribeResult");
        jmethodID res_ctor = env->GetMethodID(res_cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;ZLjava/lang/String;JJJJJ)V");
        jobject res_obj = env->NewObject(res_cls, res_ctor, env->NewStringUTF(res.language.c_str()), env->NewStringUTF(res.text.c_str()), res.success, env->NewStringUTF(res.error_msg.c_str()), res.t_load_ms, res.t_mel_ms, res.t_encode_ms, res.t_decode_ms, res.t_total_ms);

        // release resources
        env->ReleaseFloatArrayElements(samples, samples_ptr, 0);
        env->ReleaseStringUTFChars(lang, lang_str);

        return res_obj;
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_Qwen3ASR
     * Method:    transcribeFile
     * Signature: (Ljava/lang/String;Lio/github/jaffe2718/qwen3asr4j/TranscribeParams;)Lio/github/jaffe2718/qwen3asr4j/TranscribeResult;
     */
    JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_Qwen3ASR_transcribeFile(JNIEnv *env, jobject thiz, jstring audioPath, jobject params) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (!qwen3_asr_map.contains(ctx_id)) {
            env->ThrowNew(env->FindClass("java/lang/NullPointerException"), ("Qwen3ASR<" + std::to_string(ctx_id) + "> context not loaded").c_str());
            return nullptr;
        }
        // get model context (reference, not copy)
        qwen3_asr::Qwen3ASR &ctx = qwen3_asr_map[ctx_id];
        // get language
        const auto lang = (jstring) env->GetObjectField(params, env->GetFieldID(env->GetObjectClass(params), "language", "Ljava/lang/String;"));
        const char * lang_str = env->GetStringUTFChars(lang, nullptr);
        // audio path
        const char * audio_path = env->GetStringUTFChars(audioPath, nullptr);
        FILE *fp = fopen(audio_path, "rb");
        if (!fp) {
            env->ThrowNew(env->FindClass("java/io/FileNotFoundException"), "Failed to open audio file");
            return nullptr;
        }
        fclose(fp);
        // create transcribe_params
        qwen3_asr::transcribe_params t_params {
            env->GetIntField(params, env->GetFieldID(env->GetObjectClass(params), "maxTokens", "I")),
            lang_str,
            env->GetIntField(params, env->GetFieldID(env->GetObjectClass(params), "nThreads", "I")),
            (bool) env->GetBooleanField(params, env->GetFieldID(env->GetObjectClass(params), "printProgress", "Z")),
            (bool) env->GetBooleanField(params, env->GetFieldID(env->GetObjectClass(params), "printTiming", "Z"))
        };

        qwen3_asr::transcribe_result res = ctx.transcribe(audio_path, t_params);

        // create TranscribeResult object
        jclass res_cls = env->FindClass("io/github/jaffe2718/qwen3asr4j/result/TranscribeResult");
        jmethodID res_ctor = env->GetMethodID(res_cls, "<init>", "(Ljava/lang/String;Ljava/lang/String;ZLjava/lang/String;JJJJJ)V");
        jobject res_obj = env->NewObject(res_cls, res_ctor, env->NewStringUTF(res.language.c_str()), env->NewStringUTF(res.text.c_str()), res.success, env->NewStringUTF(res.error_msg.c_str()), res.t_load_ms, res.t_mel_ms, res.t_encode_ms, res.t_decode_ms, res.t_total_ms);

        // release resources
        env->ReleaseStringUTFChars(lang, lang_str);

        return res_obj;
    }

/*===================================io.github.jaffe2718.qwen3asr4j.ForcedAligner===================================*/

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_
     * Method:    getError
     * Signature: ()Ljava/lang/String;
     */
    JNIEXPORT jstring JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_getError(JNIEnv *env, jobject thiz) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (forced_aligner_map.contains(ctx_id)) {
            qwen3_asr::ForcedAligner &ctx = forced_aligner_map[ctx_id];
            return env->NewStringUTF(ctx.get_error().c_str());
        }
        return env->NewStringUTF("");
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_ForcedAligner
     * Method:    isLoaded
     * Signature: ()Z
     */
    JNIEXPORT jboolean JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_isLoaded(JNIEnv *env, jobject thiz) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        return forced_aligner_map.contains(ctx_id) && forced_aligner_map[ctx_id].is_loaded();
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_ForcedAligner
     * Method:    getHparams
     * Signature: ()Ljava/util/Map;
     */
    JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_getHparams(JNIEnv *env, jobject thiz) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (forced_aligner_map.contains(ctx_id)) {
            // get model context (reference, not copy)
            qwen3_asr::ForcedAligner &ctx = forced_aligner_map[ctx_id];
            // get hparams
            qwen3_asr::forced_aligner_hparams hparams = ctx.get_hparams();

            // Create a new HashMap
            jclass hash_map_class = env->FindClass("java/util/HashMap");
            jmethodID hash_map_init = env->GetMethodID(hash_map_class, "<init>", "()V");
            jobject hash_map = env->NewObject(hash_map_class, hash_map_init);

            // Add config entries to the map
            jmethodID hash_map_put = env->GetMethodID(hash_map_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
            // Add config entries to the map
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_encoder_layers"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_encoder_layers));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_d_model"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_d_model));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_attention_heads"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_attention_heads));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_ffn_dim"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_ffn_dim));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_num_mel_bins"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_num_mel_bins));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_conv_channels"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_conv_channels));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_layer_norm_eps"), env->NewObject(env->FindClass("java/lang/Float"), env->GetMethodID(env->FindClass("java/lang/Float"), "<init>", "(F)V"), hparams.audio_layer_norm_eps));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_decoder_layers"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.text_decoder_layers));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_hidden_size"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.text_hidden_size));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_attention_heads"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.text_attention_heads));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_kv_heads"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.text_kv_heads));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_intermediate_size"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.text_intermediate_size));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_head_dim"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.text_head_dim));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_rms_norm_eps"), env->NewObject(env->FindClass("java/lang/Float"), env->GetMethodID(env->FindClass("java/lang/Float"), "<init>", "(F)V"), hparams.text_rms_norm_eps));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("text_rope_theta"), env->NewObject(env->FindClass("java/lang/Float"), env->GetMethodID(env->FindClass("java/lang/Float"), "<init>", "(F)V"), hparams.text_rope_theta));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("vocab_size"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.vocab_size));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("classify_num"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.classify_num));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("timestamp_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.timestamp_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_start_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_start_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_end_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_end_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("audio_pad_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.audio_pad_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("pad_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.pad_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("eos_token_id"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.eos_token_id));
            env->CallObjectMethod(hash_map, hash_map_put, env->NewStringUTF("timestamp_segment_time_ms"), env->NewObject(env->FindClass("java/lang/Integer"), env->GetMethodID(env->FindClass("java/lang/Integer"), "<init>", "(I)V"), hparams.timestamp_segment_time_ms));

            return hash_map;

        }

        return nullptr;
    }

     /**
     * Class:     io_github_jaffe2718_qwen3asr4j_ForcedAligner
     * Method:    load
     * Signature: (Ljava/lang/String;)V
     */
     JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_load(JNIEnv *env, jobject thiz, jstring modelPath) {
        const char* model_path = env->GetStringUTFChars(modelPath, nullptr);
        if (model_path == nullptr) {
            env->ThrowNew(env->FindClass("java/io/FileNotFoundException"), "Model path is null");
            return;
        }

        // Create new ForcedAligner context
        int ctx_id = new_forced_aligner_id();
        forced_aligner_map[ctx_id] = qwen3_asr::ForcedAligner();

        // Load model
        const bool success = forced_aligner_map[ctx_id].load_model(model_path);
        slf4j_info(env, thiz, ("ForcedAligner<" + std::to_string(ctx_id) + "> model load state: " + (success ? "success" : "failed")).c_str());

        // Release Java string
        env->ReleaseStringUTFChars(modelPath, model_path);

        if (!success) {
            env->ThrowNew(env->FindClass("java/io/FileNotFoundException"), ("ForcedAligner model load failed: " + forced_aligner_map[ctx_id].get_error()).c_str());
            forced_aligner_map.erase(ctx_id);
            return;
        }
        env->SetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"), ctx_id);
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_ForcedAligner
     * Method:    free
     * Signature: ()V
     */
     JNIEXPORT void JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_free(JNIEnv *env, jobject thiz) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (forced_aligner_map.contains(ctx_id)) {
            forced_aligner_map.erase(ctx_id);
        }
        slf4j_info(env, thiz, ("ForcedAligner<" + std::to_string(ctx_id) + "> context freed").c_str());
    }

    /**
     * Class:     io_github_jaffe2718_qwen3asr4j_ForcedAligner
     * Method:    align
     * Signature: ([FIILjava/lang/String;Ljava/lang/String;)Lio/github/jaffe2718/qwen3asr4j/result/AlignmentResult;
     */
    JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_align(JNIEnv *env, jobject thiz, jfloatArray samples, jint n_samples, jstring text, jstring language) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (forced_aligner_map.contains(ctx_id)) {
            qwen3_asr::ForcedAligner &ctx = forced_aligner_map[ctx_id];
            jfloat *samples_ptr = env->GetFloatArrayElements(samples, nullptr);
            std::vector samples_vector(samples_ptr, samples_ptr + n_samples);
            env->ReleaseFloatArrayElements(samples, samples_ptr, 0);
            const char* language_ptr = env->GetStringUTFChars(language, nullptr);
            const char* text_ptr = env->GetStringUTFChars(text, nullptr);
            std::string text_str = std::string(text_ptr);
            std::string language_str = std::string(language_ptr);
            qwen3_asr::alignment_result result = ctx.align(samples_vector.data(), n_samples, text_str, language_str);
            env->ReleaseStringUTFChars(text, text_ptr);
            env->ReleaseStringUTFChars(language, language_ptr);

            // create result object
            jclass result_class = env->FindClass("io/github/jaffe2718/qwen3asr4j/result/AlignmentResult");
            jmethodID result_constructor = env->GetMethodID(result_class, "<init>", "([Lio/github/jaffe2718/qwen3asr4j/result/AlignedWord;ZLjava/lang/String;JJJJ)V");
            jobjectArray words_array = env->NewObjectArray(result.words.size(), env->FindClass("io/github/jaffe2718/qwen3asr4j/result/AlignedWord"), nullptr);
            for (size_t i = 0; i < result.words.size(); i++) {
                jclass aligned_word_class = env->FindClass("io/github/jaffe2718/qwen3asr4j/result/AlignedWord");
                jmethodID aligned_word_constructor = env->GetMethodID(aligned_word_class, "<init>", "(Ljava/lang/String;FF)V");
                jobject aligned_word_obj = env->NewObject(aligned_word_class, aligned_word_constructor,
                    env->NewStringUTF(result.words[i].word.c_str()),
                    result.words[i].start,
                    result.words[i].end
                );
                env->SetObjectArrayElement(words_array, i, aligned_word_obj);
            }
            return env->NewObject(result_class, result_constructor,
                words_array,
                result.success,
                env->NewStringUTF(result.error_msg.c_str()),
                result.t_mel_ms,
                result.t_encode_ms,
                result.t_decode_ms,
                result.t_total_ms
            );
        }
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), ("ForcedAligner<" + std::to_string(ctx_id) + "> not loaded").c_str());
        return nullptr;
    }

    JNIEXPORT jobject JNICALL Java_io_github_jaffe2718_qwen3asr4j_ForcedAligner_alignFile(JNIEnv *env, jobject thiz, jstring audioPath, jstring text, jstring language) {
        jint ctx_id = env->GetIntField(thiz, env->GetFieldID(env->GetObjectClass(thiz), "ctxId", "I"));
        if (forced_aligner_map.contains(ctx_id)) {
            qwen3_asr::ForcedAligner &ctx = forced_aligner_map[ctx_id];
            const char * audio_path_ptr = env->GetStringUTFChars(audioPath, nullptr);
            const char* language_ptr = env->GetStringUTFChars(language, nullptr);
            const char* text_ptr = env->GetStringUTFChars(text, nullptr);
            std::string audio_path_str = std::string(audio_path_ptr);
            std::string text_str = std::string(text_ptr);
            std::string language_str = std::string(language_ptr);
            FILE *fp = fopen(audio_path_ptr, "rb");
            if (!fp) {
                env->ThrowNew(env->FindClass("java/io/FileNotFoundException"), "Failed to open audio file");
                env->ReleaseStringUTFChars(audioPath, audio_path_ptr);
                env->ReleaseStringUTFChars(language, language_ptr);
                env->ReleaseStringUTFChars(text, text_ptr);
                return nullptr;
            }
            fclose(fp);
            qwen3_asr::alignment_result result = ctx.align(audio_path_str, text_str, language_str);
            env->ReleaseStringUTFChars(audioPath, audio_path_ptr);
            env->ReleaseStringUTFChars(language, language_ptr);
            env->ReleaseStringUTFChars(text, text_ptr);

            // create result object
            jclass result_class = env->FindClass("io/github/jaffe2718/qwen3asr4j/result/AlignmentResult");
            jmethodID result_constructor = env->GetMethodID(result_class, "<init>", "([Lio/github/jaffe2718/qwen3asr4j/result/AlignedWord;ZLjava/lang/String;JJJJ)V");
            jobjectArray words_array = env->NewObjectArray(result.words.size(), env->FindClass("io/github/jaffe2718/qwen3asr4j/result/AlignedWord"), nullptr);
            for (size_t i = 0; i < result.words.size(); i++) {
                jclass aligned_word_class = env->FindClass("io/github/jaffe2718/qwen3asr4j/result/AlignedWord");
                jmethodID aligned_word_constructor = env->GetMethodID(aligned_word_class, "<init>", "(Ljava/lang/String;FF)V");
                jobject aligned_word_obj = env->NewObject(aligned_word_class, aligned_word_constructor,
                    env->NewStringUTF(result.words[i].word.c_str()),
                    result.words[i].start,
                    result.words[i].end
                );
                env->SetObjectArrayElement(words_array, i, aligned_word_obj);
            }
            return env->NewObject(result_class, result_constructor,
                words_array,
                result.success,
                env->NewStringUTF(result.error_msg.c_str()),
                result.t_mel_ms,
                result.t_encode_ms,
                result.t_decode_ms,
                result.t_total_ms
            );
        }
        env->ThrowNew(env->FindClass("java/lang/NullPointerException"), ("ForcedAligner<" + std::to_string(ctx_id) + "> not loaded").c_str());
        return nullptr;
    }

#ifdef __cplusplus
}
#endif