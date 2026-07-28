#include <qwen3_asr.h>
#include <forced_aligner.h>
#include <iostream>
#include <ostream>

std::string test_qwen3asr(std::string model) {
    qwen3_asr::Qwen3ASR qwen3asr;
    qwen3asr.load_model(model);
    qwen3_asr::transcribe_result result = qwen3asr.transcribe("samples/jfk.wav");
    std::cout << result.text << std::endl;
    return result.text;
}

void test_forced_aligner(std::string & text) {
    qwen3_asr::ForcedAligner aligner;
    aligner.load_model("models/qwen3-forcedaligner-0.6b-f16.gguf");
    qwen3_asr::alignment_result result = aligner.align("samples/jfk.wav", text);
    for (auto & a_word : result.words) {
        std::cout << "(" << a_word.word << ", " << a_word.start << " --> " << a_word.end << ")" << std::endl;
    }
}

int main() {
    test_qwen3asr("models/qwen3-asr-1.7b-q4_1.gguf");
    std::string text06b = test_qwen3asr("models/qwen3-asr-0.6b-q8_0.gguf");
    test_forced_aligner(text06b);
    return 0;
}