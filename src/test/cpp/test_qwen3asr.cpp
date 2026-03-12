#include <qwen3_asr.h>
#include <iostream>

using namespace std;

int main() {
    cout << "qwen3asr test" << endl;
    qwen3_asr::Qwen3ASR asr;
    cout << "[test] (load-model) qwen3-asr-0.6b-f16.gguf" << endl;
    asr.load_model("qwen3-asr-0.6b-f16.gguf");
    cout << "[test] (load-model) asr.is_loaded() = " << asr.is_loaded() << endl;
    cout << "[test] (load-model) asr.get_error() = " << asr.get_error() << endl;
    cout << "[test] (load-model) DONE" << endl;
    cout << "[test] (transcribe) qwen3-asr-0.6b-f16.gguf" << endl;
    string audio_file = "samples/jfk.wav";
    cout << "[test] (transcribe) audio_file = " << audio_file << endl;
    qwen3_asr::transcribe_result result = asr.transcribe(audio_file);
    cout << "[test] (transcribe) result.success = " << result.success << endl;
    cout << "[test] (transcribe) result.error_msg = " << result.error_msg << endl;
    cout << "[test] (transcribe) result.text = " << result.text << endl;
    cout << "[test] (transcribe) result.tokens = " << result.tokens.size() << endl;
    cout << "[test] (transcribe) result.t_decode_ms = " << result.t_decode_ms << endl;
    cout << "[test] (transcribe) result.t_total_ms = " << result.t_total_ms << endl;
    cout << "[test] (transcribe) DONE" << endl;
}