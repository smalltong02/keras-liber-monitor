#pragma once
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <thread>
#include <functional>
#include <memory>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <atomic>

namespace cchips {

    class CWord2Vec {
    public:
        typedef float real;                    // Precision of float numbers

        struct vocab_word {
            std::uint32_t cn;
            std::vector<int> point;
            std::vector<char> code;
            std::string word;
            int codelen;
        };

        CWord2Vec() = default;
        ~CWord2Vec() = default;
        //static CWord2Vec& GetInstance()
        //{
        //    static CWord2Vec m_instance;
        //    return m_instance;
        //}

        void ClearAllCache() {
            m_syn0.clear();
            m_syn1.clear();
            m_syn1neg.clear();
            m_expTable.clear();
            m_table.clear();
            m_vocab_hash.clear();
            m_vocab.clear();
            return;
        }

        void Initialize(const std::string& train_file, const std::string& output_file, const std::string& save_vocab_file, const std::string& read_vocab_file, std::uint32_t size = 100, \
            std::uint32_t window = 5, real threshold = 1e-3, std::uint32_t hs = 0, std::uint32_t negative = 5, std::uint32_t threads = 1, std::uint32_t iter = 5, std::uint32_t min_count = 0, \
            real alpha = 0.025, std::uint32_t classes = 0, std::uint32_t debug = 2, std::uint32_t binary = 0, std::uint32_t cbow = 1) {
            m_bvalid = false;
            m_train_file = train_file;
            m_output_file = output_file;
            m_save_vocab_file = save_vocab_file;
            m_read_vocab_file = read_vocab_file;
            m_debug_mode = debug;
            m_binary = binary;
            m_cbow = cbow;
            m_alpha = alpha;
            m_window = window;
            m_sample = threshold;
            m_hs = hs;
            m_negative = negative;
            m_num_threads = threads;
            m_iter = iter;
            m_min_count = min_count;
            m_classes = classes;
            m_layer1_size = size;

            m_vocab.resize(m_vocab_max_size);
            m_vocab_hash.resize(_vocab_hash_size);
            m_expTable.resize(_exp_table_size + 1);
            if (m_expTable.size() != _exp_table_size + 1)
                return;
            for (int i = 0; i < _exp_table_size; i++) {
                m_expTable[i] = exp((i / (real)_exp_table_size * 2 - 1) * _max_exp); // Precompute the exp() table
                m_expTable[i] = m_expTable[i] / (m_expTable[i] + 1);                   // Precompute f(x) = x / (x + 1)
            }
            m_bvalid = true;
            return;
        }

        void TrainModel() {
            if (!m_bvalid)
                return;
            long a, b, c, d;
            m_worker_thread.resize(m_num_threads);
            //printf("Starting training using file %s\n", m_train_file.c_str());
            m_starting_alpha = m_alpha;
            if (m_read_vocab_file.length()) ReadVocab(); else LearnVocabFromTrainFile();
            if (m_save_vocab_file.length()) SaveVocab();
            if (!m_output_file.length()) return;
            InitNet();
            if (m_negative > 0) InitUnigramTable();
            m_start = clock();

            for (a = 0; a < m_num_threads; a++) {
                auto thread = std::make_unique<std::thread>(std::bind(&CWord2Vec::TrainModelThread, this, (void*)a));
                if (thread) {
                    m_worker_thread.push_back(std::move(thread));
                }
            }
            for (auto& thread : m_worker_thread) {
                if (thread) {
                    thread->join();
                }
            }
            //printf("\n\nGenerating file: %s ...\n", m_output_file.c_str());
            do {
                std::fstream iofile(m_output_file, std::ios::out | std::ios::binary);
                if (!iofile.is_open()) {
                    break;
                }
                if (m_classes == 0) {
                    // Save the word vectors
                    //iofile << m_vocab_size << " " << m_layer1_size;
                    for (a = 0; a < m_vocab_size; a++) {
                        if (!m_vocab[a].word.length()) continue;
                        iofile << m_vocab[a].word.c_str() << " ";
                        if (m_binary) for (b = 0; b < m_layer1_size; b++) {
                            iofile << m_syn0[a * m_layer1_size + b];
                        }
                        else {
                            for (b = 0; b < m_layer1_size; b++) {
                                iofile << m_syn0[a * m_layer1_size + b] << " ";
                            }
                        }
                        iofile << std::endl;
                    }
                }
                else {
                    // Run K-means on the word vectors
                    int clcn = (int)m_classes, iter = 10, closeid;
                    int* centcn = (int*)malloc(m_classes * sizeof(int));
                    int* cl = (int*)calloc(m_vocab_size, sizeof(int));
                    real closev, x;
                    real* cent = (real*)calloc(m_classes * m_layer1_size, sizeof(real));
                    for (a = 0; a < m_vocab_size; a++) cl[a] = a % clcn;
                    for (a = 0; a < iter; a++) {
                        for (b = 0; b < clcn * m_layer1_size; b++) cent[b] = 0;
                        for (b = 0; b < clcn; b++) centcn[b] = 1;
                        for (c = 0; c < m_vocab_size; c++) {
                            for (d = 0; d < m_layer1_size; d++) cent[m_layer1_size * cl[c] + d] += m_syn0[c * m_layer1_size + d];
                            centcn[cl[c]]++;
                        }
                        for (b = 0; b < clcn; b++) {
                            closev = 0;
                            for (c = 0; c < m_layer1_size; c++) {
                                cent[m_layer1_size * b + c] /= centcn[b];
                                closev += cent[m_layer1_size * b + c] * cent[m_layer1_size * b + c];
                            }
                            closev = sqrt(closev);
                            for (c = 0; c < m_layer1_size; c++) cent[m_layer1_size * b + c] /= closev;
                        }
                        for (c = 0; c < m_vocab_size; c++) {
                            closev = -10;
                            closeid = 0;
                            for (d = 0; d < clcn; d++) {
                                x = 0;
                                for (b = 0; b < m_layer1_size; b++) x += cent[m_layer1_size * d + b] * m_syn0[c * m_layer1_size + b];
                                if (x > closev) {
                                    closev = x;
                                    closeid = d;
                                }
                            }
                            cl[c] = closeid;
                        }
                    }
                    // Save the K-means classes
                    for (a = 0; a < m_vocab_size; a++) {
                        iofile << m_vocab[a].word << " " << cl[a] << std::endl;
                    }
                    free(centcn);
                    free(cent);
                    free(cl);
                }
                iofile.close();
            } while (0);

            //printf("Task finished.\n");
            return;
        }

    private:
        void InitUnigramTable() {
            int a, i;
            double train_words_pow = 0;
            double d1, power = 0.75;
            m_table.resize(m_table_size);
            if (m_table.size() != m_table_size)
                return;
            for (a = 0; a < m_vocab.size(); a++) {
                train_words_pow += pow(m_vocab[a].cn, power);
            }
            i = 0;
            d1 = pow(m_vocab[i].cn, power) / train_words_pow;
            for (a = 0; a < m_vocab.size(); a++) {
                m_table[a] = i;
                if (a / (double)m_table.size() > d1) {
                    i++;
                    d1 += pow(m_vocab[i].cn, power) / train_words_pow;
                }
                if (i >= m_vocab.size()) i = m_vocab.size() - 1;
            }
        }

        // Reads a single word from a file, assuming space + tab + EOL to be word boundaries
        void ReadWord(char* word, std::fstream& file, char* eof) {
            int a = 0, ch;
            std::lock_guard<std::mutex> lock(m_safe_mutex);
            while (1) {
                ch = file.get();
                if (ch == EOF) {
                    *eof = 1;
                    break;
                }
                if (ch == 13) continue;
                if ((ch == ' ') || (ch == '\t') || (ch == '\n')) {
                    if (a > 0) {
                        if (ch == '\n') {
                            file.putback(ch);
                        }
                        break;
                    }
                    if (ch == '\n') {
                        strcpy_s(word, sizeof("</s>") + 1, (char*)"</s>");
                        return;
                    }
                    else continue;
                }
                word[a] = ch;
                a++;
                if (a >= _max_string - 1) a--;   // Truncate too long words
            }
            word[a] = 0;
        }
        // Returns hash value of a word
        int GetWordHash(const std::string& word) {
            unsigned long long a, hash = 0;
            for (a = 0; a < word.length(); a++) hash = hash * 257 + word[a];
            hash = hash % _vocab_hash_size;
            return hash;
        }
        // Returns position of a word in the vocabulary; if the word is not found, returns -1
        int SearchVocab(char* word) {
            unsigned int hash = GetWordHash(word);
            while (1) {
                if (m_vocab_hash[hash] == -1) return -1;
                if (!strcmp(word, m_vocab[m_vocab_hash[hash]].word.c_str())) return m_vocab_hash[hash];
                hash = (hash + 1) % _vocab_hash_size;
            }
            return -1;
        }
        // Reads a word and returns its index in the vocabulary
        int ReadWordIndex(std::fstream& file, char* eof) {
            char word[_max_string], eof_l = 0;
            ReadWord(word, file, &eof_l);
            if (eof_l) {
                *eof = 1;
                return -1;
            }
            return SearchVocab(word);
        }
        // Adds a word to the vocabulary
        int AddWordToVocab(char* word) {
            unsigned int hash, length = strlen(word);
            if (length > _max_string) length = _max_string;
            m_vocab[m_vocab_size].word = std::string(word, length);
            m_vocab[m_vocab_size].cn = 0;
            m_vocab_size++;
            // Reallocate memory if needed
            if (m_vocab_size + 2 >= m_vocab_max_size) {
                m_vocab_max_size += 1000;
                m_vocab.resize(m_vocab_max_size/* * sizeof(CWord2Vec::vocab_word)*/);
                if (m_vocab.size() != m_vocab_max_size/* * sizeof(CWord2Vec::vocab_word)*/)
                    return -1;
            }
            hash = GetWordHash(word);
            while (m_vocab_hash[hash] != -1) hash = (hash + 1) % _vocab_hash_size;
            m_vocab_hash[hash] = m_vocab_size - 1;
            return m_vocab_size - 1;
        }
        // Used later for sorting by word counts
        static bool VocabCompare(const CWord2Vec::vocab_word& a, const CWord2Vec::vocab_word& b) {
            std::int32_t l = b.cn - a.cn;
            if (l > 0) return true;
            return false;
        }
        // Sorts the vocabulary by frequency using word counts
        void SortVocab() {
            int a, size;
            int val;
            unsigned int hash;
            // Sort the vocabulary and keep </s> at the first position
            auto it = m_vocab.begin() + m_vocab_size;
            std::sort(m_vocab.begin(), it, VocabCompare);
            for (a = 0; a < _vocab_hash_size; a++) m_vocab_hash[a] = -1;
            size = m_vocab_size;
            m_train_words = 0;
            for (a = 0, val = 0; a < size; a++) {
                // Words occuring less than min_count times will be discarded from the vocab
                if ((m_vocab[a].cn < m_min_count) && (a != 0)) {
                    //m_vocab_size--;
                    m_vocab[a].word.clear();
                }
                else {
                    // Hash will be re-computed, as after the sorting it is not actual
                    hash = GetWordHash(m_vocab[a].word);
                    while (m_vocab_hash[hash] != -1) hash = (hash + 1) % _vocab_hash_size;
                    m_vocab_hash[hash] = a;
                    m_train_words += m_vocab[a].cn;
                }
            }
            // Allocate memory for the binary tree construction
            for (a = 0; a < m_vocab_size; a++) {
                m_vocab[a].code.resize(_max_code_length);
                m_vocab[a].point.resize(_max_code_length);
            }
        }
        // Reduces the vocabulary by removing infrequent tokens
        void ReduceVocab() {
            int a, b = 0;
            unsigned int hash;
            for (a = 0; a < m_vocab_size; a++) if (m_vocab[a].cn > m_min_reduce) {
                m_vocab[b].cn = m_vocab[a].cn;
                m_vocab[b].word = m_vocab[a].word;
                b++;
            }
            else {
                m_vocab[a].word.clear();
            }
            m_vocab_size = b;
            for (a = 0; a < _vocab_hash_size; a++) m_vocab_hash[a] = -1;
            for (a = 0; a < m_vocab_size; a++) {
                // Hash will be re-computed, as it is not actual
                hash = GetWordHash(m_vocab[a].word);
                while (m_vocab_hash[hash] != -1) hash = (hash + 1) % _vocab_hash_size;
                m_vocab_hash[hash] = a;
            }
            fflush(stdout);
            m_min_reduce++;
        }
        // Create binary Huffman tree using the word counts
        // Frequent words will have short uniqe binary codes
        void CreateBinaryTree() {
            long long a, b, i, min1i, min2i, pos1, pos2, point[_max_code_length];
            char code[_max_code_length];
            long long* count = (long long*)calloc(m_vocab_size * 2 + 1, sizeof(long long));
            long long* binary = (long long*)calloc(m_vocab_size * 2 + 1, sizeof(long long));
            long long* parent_node = (long long*)calloc(m_vocab_size * 2 + 1, sizeof(long long));
            for (a = 0; a < m_vocab_size; a++) count[a] = m_vocab[a].cn;
            for (a = m_vocab_size; a < m_vocab_size * 2; a++) count[a] = 1e15;
            pos1 = m_vocab_size - 1;
            pos2 = m_vocab_size;
            // Following algorithm constructs the Huffman tree by adding one node at a time
            for (a = 0; a < m_vocab_size - 1; a++) {
                // First, find two smallest nodes 'min1, min2'
                if (pos1 >= 0) {
                    if (count[pos1] < count[pos2]) {
                        min1i = pos1;
                        pos1--;
                    }
                    else {
                        min1i = pos2;
                        pos2++;
                    }
                }
                else {
                    min1i = pos2;
                    pos2++;
                }
                if (pos1 >= 0) {
                    if (count[pos1] < count[pos2]) {
                        min2i = pos1;
                        pos1--;
                    }
                    else {
                        min2i = pos2;
                        pos2++;
                    }
                }
                else {
                    min2i = pos2;
                    pos2++;
                }
                count[m_vocab_size + a] = count[min1i] + count[min2i];
                parent_node[min1i] = m_vocab_size + a;
                parent_node[min2i] = m_vocab_size + a;
                binary[min2i] = 1;
            }
            // Now assign binary code to each vocabulary word
            for (a = 0; a < m_vocab_size; a++) {
                b = a;
                i = 0;
                while (1) {
                    code[i] = binary[b];
                    point[i] = b;
                    i++;
                    b = parent_node[b];
                    if (b == m_vocab_size * 2 - 2) break;
                }
                m_vocab[a].codelen = i;
                m_vocab[a].point[0] = m_vocab_size - 2;
                for (b = 0; b < i; b++) {
                    m_vocab[a].code[i - b - 1] = code[b];
                    m_vocab[a].point[i - b] = point[b] - m_vocab_size;
                }
            }
            free(count);
            free(binary);
            free(parent_node);
        }
        void LearnVocabFromTrainFile() {
            char word[_max_string], eof = 0;
            long long a, i, wc = 0;
            for (a = 0; a < _vocab_hash_size; a++) m_vocab_hash[a] = -1;
            std::fstream infile;
            infile.open(m_train_file, std::ios::in | std::ios::binary | std::ios::ate);
            if (!infile.is_open()) {
                printf("ERROR: training data file not found!\n");
                return;
            }
            m_file_size = infile.tellg();
            infile.seekg(0, std::ios::beg);
            m_vocab_size = 0;
            AddWordToVocab((char*)"</s>");
            while (1) {
                ReadWord(word, infile, &eof);
                if (eof) break;
                m_train_words++;
                wc++;
                if ((m_debug_mode > 1) && (wc >= 1000000)) {
                    //printf("%lldM%c", m_train_words / 1000000, 13);
                    fflush(stdout);
                    wc = 0;
                }
                i = SearchVocab(word);
                if (i == -1) {
                    m_real_train_words++;
                    a = AddWordToVocab(word);
                    m_vocab[a].cn = 1;
                }
                else m_vocab[i].cn++;
                if (m_vocab_size > _vocab_hash_size * 0.7) ReduceVocab();
            }
            SortVocab();
            if (m_debug_mode > 0) {
                //printf("Vocab size: %lld\n", m_vocab_size);
                //printf("Words in train file: %lld\n", m_train_words);
                printf("Words in train set: %lld\n", m_real_train_words);
            }
            infile.close();
        }
        void SaveVocab() {
            long long i;
            std::ofstream outfile;
            outfile.open(m_output_file, std::ios::out | std::ios::binary);
            if (!outfile.is_open()) {
                printf("ERROR: output data file not create!\n");
                return;
            }
            for (i = 0; i < m_vocab_size; i++) {
                outfile << m_vocab[i].word << " " << m_vocab[i].cn << std::endl;
            }
            outfile.close();
        }
        void ReadVocab() {
            long long a, i = 0;
            char c, eof = 0;
            char word[_max_string];
            std::fstream iofile;
            iofile.open(m_read_vocab_file, std::ios::in | std::ios::out | std::ios::binary);
            if (!iofile.is_open()) {
                printf("Vocabulary file not found\n");
                return;
            }
            for (a = 0; a < _vocab_hash_size; a++) m_vocab_hash[a] = -1;
            m_vocab_size = 0;
            while (1) {
                ReadWord(word, iofile, &eof);
                if (eof) break;
                a = AddWordToVocab(word);
                iofile >> m_vocab[a].cn >> c;
                i++;
            }
            SortVocab();
            if (m_debug_mode > 0) {
                printf("Vocab size: %lld\n", m_vocab_size);
                printf("Words in train file: %lld\n", m_train_words);
                printf("Words in train set: %lld\n", m_real_train_words);
            }
            iofile.close();
            std::fstream trainfile;
            trainfile.open(m_train_file, std::ios::in | std::ios::binary | std::ios::ate);
            if (!trainfile.is_open()) {
                printf("ERROR: training data file not found!\n");
                return;
            }
            m_file_size = iofile.tellg();
            iofile.close();
        }
        void InitNet() {
            long long a, b;
            unsigned long long next_random = 1;
            m_syn0.resize((long long)m_vocab_size * m_layer1_size);
            if (m_syn0.size() != (long long)m_vocab_size * m_layer1_size) {
                printf("Memory allocation failed\n");
                return;
            }
            if (m_hs) {
                m_syn1.resize((long long)m_vocab_size * m_layer1_size);
                if (m_syn1.size() != (long long)m_vocab_size * m_layer1_size) {
                    printf("Memory allocation failed\n");
                    return;
                }
                for (a = 0; a < m_vocab_size; a++) for (b = 0; b < m_layer1_size; b++)
                    m_syn1[a * m_layer1_size + b] = 0;
            }
            if (m_negative > 0) {
                m_syn1neg.resize((long long)m_vocab_size * m_layer1_size);
                if (m_syn1neg.size() != (long long)m_vocab_size * m_layer1_size) {
                    printf("Memory allocation failed\n");
                    return;
                }
                for (a = 0; a < m_vocab_size; a++) for (b = 0; b < m_layer1_size; b++)
                    m_syn1neg[a * m_layer1_size + b] = 0;
            }
            for (a = 0; a < m_vocab_size; a++) for (b = 0; b < m_layer1_size; b++) {
                next_random = next_random * (unsigned long long)25214903917 + 11;
                m_syn0[a * m_layer1_size + b] = (((next_random & 0xFFFF) / (real)65536) - 0.5) / m_layer1_size;
            }
            CreateBinaryTree();
        }
        void TrainModelThread(void* id) {
            long long a, b, d, cw, word, last_word, sentence_length = 0, sentence_position = 0;
            long long word_count = 0, last_word_count = 0, sen[_max_sentence_length + 1];
            long long l1, l2, c, target, label, local_iter = m_iter;
            unsigned long long next_random = (long long)id;
            char eof = 0;
            real f, g;
            clock_t now;
            real* neu1 = (real*)calloc(m_layer1_size, sizeof(real));
            real* neu1e = (real*)calloc(m_layer1_size, sizeof(real));
            std::fstream iofile;
            iofile.open(m_train_file, std::ios::in | std::ios::binary);
            if (!iofile.is_open()) {
                return;
            }
            iofile.seekg(m_file_size / (long long)m_num_threads * (long long)id, std::ios::beg);
            while (1) {
                if (word_count - last_word_count > 10000) {
                    m_word_count_actual += word_count - last_word_count;
                    last_word_count = word_count;
                    if ((m_debug_mode > 1)) {
                        now = clock();
                        printf("%cAlpha: %f  Progress: %.2f%%  Words/thread/sec: %.2fk  ", 13, m_alpha,
                            m_word_count_actual / (real)(m_iter * m_train_words + 1) * 100,
                            m_word_count_actual / ((real)(now - m_start + 1) / (real)CLOCKS_PER_SEC * 1000));
                        fflush(stdout);
                    }
                    m_alpha = m_starting_alpha * (1 - m_word_count_actual / (real)(m_iter * m_train_words + 1));
                    if (m_alpha < m_starting_alpha * 0.0001) m_alpha = m_starting_alpha * 0.0001;
                }
                if (sentence_length == 0) {
                    while (1) {
                        word = ReadWordIndex(iofile, &eof);
                        if (eof) break;
                        if (word == -1) continue;
                        word_count++;
                        if (word == 0) break;
                        // The subsampling randomly discards frequent words while keeping the ranking same
                        if (m_sample > 0) {
                            real ran = (sqrt(m_vocab[word].cn / (m_sample * m_train_words)) + 1) * (m_sample * m_train_words) / m_vocab[word].cn;
                            next_random = next_random * (unsigned long long)25214903917 + 11;
                            if (ran < (next_random & 0xFFFF) / (real)65536) continue;
                        }
                        sen[sentence_length] = word;
                        sentence_length++;
                        if (sentence_length >= _max_sentence_length) break;
                    }
                    sentence_position = 0;
                }
                if (eof || (word_count > m_train_words / m_num_threads)) {
                    m_word_count_actual += word_count - last_word_count;
                    local_iter--;
                    if (local_iter == 0) break;
                    word_count = 0;
                    last_word_count = 0;
                    sentence_length = 0;
                    iofile.seekg(m_file_size / (long long)m_num_threads * (long long)id, std::ios::beg);
                    continue;
                }
                word = sen[sentence_position];
                if (word == -1) continue;
                for (c = 0; c < m_layer1_size; c++) neu1[c] = 0;
                for (c = 0; c < m_layer1_size; c++) neu1e[c] = 0;
                next_random = next_random * (unsigned long long)25214903917 + 11;
                b = next_random % m_window;
                if (m_cbow) {  //train the cbow architecture
                    // in -> hidden
                    cw = 0;
                    for (a = b; a < m_window * 2 + 1 - b; a++) if (a != m_window) {
                        c = sentence_position - m_window + a;
                        if (c < 0) continue;
                        if (c >= sentence_length) continue;
                        last_word = sen[c];
                        if (last_word == -1) continue;
                        for (c = 0; c < m_layer1_size; c++) neu1[c] += m_syn0[c + last_word * m_layer1_size];
                        cw++;
                    }
                    if (cw) {
                        for (c = 0; c < m_layer1_size; c++) neu1[c] /= cw;
                        if (m_hs) for (d = 0; d < m_vocab[word].codelen; d++) {
                            f = 0;
                            l2 = m_vocab[word].point[d] * m_layer1_size;
                            // Propagate hidden -> output
                            for (c = 0; c < m_layer1_size; c++) f += neu1[c] * m_syn1[c + l2];
                            if (f <= -_max_exp) continue;
                            else if (f >= _max_exp) continue;
                            else f = m_expTable[(int)((f + _max_exp) * (_exp_table_size / _max_exp / 2))];
                            // 'g' is the gradient multiplied by the learning rate
                            g = (1 - m_vocab[word].code[d] - f) * m_alpha;
                            // Propagate errors output -> hidden
                            for (c = 0; c < m_layer1_size; c++) neu1e[c] += g * m_syn1[c + l2];
                            // Learn weights hidden -> output
                            for (c = 0; c < m_layer1_size; c++) m_syn1[c + l2] += g * neu1[c];
                        }
                        // NEGATIVE SAMPLING
                        if (m_negative > 0) for (d = 0; d < m_negative + 1; d++) {
                            if (d == 0) {
                                target = word;
                                label = 1;
                            }
                            else {
                                next_random = next_random * (unsigned long long)25214903917 + 11;
                                target = m_table[(next_random >> 16) % m_table_size];
                                if (target == 0) target = next_random % (m_vocab_size - 1) + 1;
                                if (target == word) continue;
                                label = 0;
                            }
                            l2 = target * m_layer1_size;
                            f = 0;
                            for (c = 0; c < m_layer1_size; c++) f += neu1[c] * m_syn1neg[c + l2];
                            if (f > _max_exp) g = (label - 1) * m_alpha;
                            else if (f < -_max_exp) g = (label - 0) * m_alpha;
                            else g = (label - m_expTable[(int)((f + _max_exp) * (_exp_table_size / _max_exp / 2))]) * m_alpha;
                            for (c = 0; c < m_layer1_size; c++) neu1e[c] += g * m_syn1neg[c + l2];
                            for (c = 0; c < m_layer1_size; c++) m_syn1neg[c + l2] += g * neu1[c];
                        }
                        // hidden -> in
                        for (a = b; a < m_window * 2 + 1 - b; a++) if (a != m_window) {
                            c = sentence_position - m_window + a;
                            if (c < 0) continue;
                            if (c >= sentence_length) continue;
                            last_word = sen[c];
                            if (last_word == -1) continue;
                            for (c = 0; c < m_layer1_size; c++) m_syn0[c + last_word * m_layer1_size] += neu1e[c];
                        }
                    }
                }
                else {  //train skip-gram
                    for (a = b; a < m_window * 2 + 1 - b; a++) if (a != m_window) {
                        c = sentence_position - m_window + a;
                        if (c < 0) continue;
                        if (c >= sentence_length) continue;
                        last_word = sen[c];
                        if (last_word == -1) continue;
                        l1 = last_word * m_layer1_size;
                        for (c = 0; c < m_layer1_size; c++) neu1e[c] = 0;
                        // HIERARCHICAL SOFTMAX
                        if (m_hs) for (d = 0; d < m_vocab[word].codelen; d++) {
                            f = 0;
                            l2 = m_vocab[word].point[d] * m_layer1_size;
                            // Propagate hidden -> output
                            for (c = 0; c < m_layer1_size; c++) f += m_syn0[c + l1] * m_syn1[c + l2];
                            if (f <= -_max_exp) continue;
                            else if (f >= _max_exp) continue;
                            else f = m_expTable[(int)((f + _max_exp) * (_exp_table_size / _max_exp / 2))];
                            // 'g' is the gradient multiplied by the learning rate
                            g = (1 - m_vocab[word].code[d] - f) * m_alpha;
                            // Propagate errors output -> hidden
                            for (c = 0; c < m_layer1_size; c++) neu1e[c] += g * m_syn1[c + l2];
                            // Learn weights hidden -> output
                            for (c = 0; c < m_layer1_size; c++) m_syn1[c + l2] += g * m_syn0[c + l1];
                        }
                        // NEGATIVE SAMPLING
                        if (m_negative > 0) for (d = 0; d < m_negative + 1; d++) {
                            if (d == 0) {
                                target = word;
                                label = 1;
                            }
                            else {
                                next_random = next_random * (unsigned long long)25214903917 + 11;
                                target = m_table[(next_random >> 16) % m_table_size];
                                if (target == 0) target = next_random % (m_vocab_size - 1) + 1;
                                if (target == word) continue;
                                label = 0;
                            }
                            l2 = target * m_layer1_size;
                            f = 0;
                            for (c = 0; c < m_layer1_size; c++) f += m_syn0[c + l1] * m_syn1neg[c + l2];
                            if (f > _max_exp) g = (label - 1) * m_alpha;
                            else if (f < -_max_exp) g = (label - 0) * m_alpha;
                            else g = (label - m_expTable[(int)((f + _max_exp) * (_exp_table_size / _max_exp / 2))]) * m_alpha;
                            for (c = 0; c < m_layer1_size; c++) neu1e[c] += g * m_syn1neg[c + l2];
                            for (c = 0; c < m_layer1_size; c++) m_syn1neg[c + l2] += g * m_syn0[c + l1];
                        }
                        // Learn weights input -> hidden
                        for (c = 0; c < m_layer1_size; c++) m_syn0[c + l1] += neu1e[c];
                    }
                }
                sentence_position++;
                if (sentence_position >= sentence_length) {
                    sentence_length = 0;
                    continue;
                }
            }
            iofile.close();
            free(neu1);
            free(neu1e);
            return;
        }

    private:
        static const std::uint32_t _max_string = 150;
        static const std::uint32_t _exp_table_size = 1000;
        static const std::int32_t _max_exp = 6;
        static const std::uint32_t _max_sentence_length = 1000;
        static const std::uint32_t _max_code_length = 40;
        static const std::uint32_t _vocab_hash_size = 30000000;

        bool m_bvalid = false;
        std::uint32_t m_binary = 0;
        std::uint32_t m_cbow = 1;
        std::uint32_t m_debug_mode = 2;
        std::uint32_t m_window = 5;
        std::uint32_t m_min_count = 5;
        std::uint32_t m_num_threads = 12;
        std::uint32_t m_min_reduce = 1;
        std::uint32_t m_hs = 0;
        std::uint32_t m_negative = 5;
        std::uint32_t m_table_size = 1e8;
        std::string m_train_file;
        std::string m_output_file;
        std::string m_save_vocab_file;
        std::string m_read_vocab_file;
        std::uint64_t m_vocab_max_size = 1000;
        std::uint64_t m_vocab_size = 0;
        std::uint64_t m_layer1_size = 100;
        std::uint64_t m_train_words = 0;
        std::uint64_t m_real_train_words = 0;
        std::atomic_uint64_t m_word_count_actual = 0;
        std::uint64_t m_iter = 5;
        std::uint64_t m_file_size = 0;
        std::uint64_t m_classes = 0;
        clock_t m_start;
        real m_alpha = 0.025;
        real m_starting_alpha = 0;
        real m_sample = 1e-3;
        //real* m_syn0 = nullptr;
        //real* m_syn1 = nullptr;
        //real* m_syn1neg = nullptr;
        //real* m_expTable = nullptr;
        std::vector<real> m_syn0;
        std::vector<real> m_syn1;
        std::vector<real> m_syn1neg;
        std::vector<real> m_expTable;
        std::vector<std::uint32_t> m_table;
        std::vector<std::uint32_t> m_vocab_hash;
        std::vector<CWord2Vec::vocab_word> m_vocab;
        std::vector<std::unique_ptr<std::thread>> m_worker_thread;
        std::mutex m_safe_mutex;
    };

//#define GetCWord2VecInstance() CWord2Vec::GetInstance()
} // namespace cchips
