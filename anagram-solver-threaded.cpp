#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <chrono>
#include <queue>
#include <thread>

unsigned long long letterHash(const std::string& word, const std::unordered_map<char, int>& prime_numbers) {

    unsigned long long hash_value = 1;

    for (int i = 0; i < word.size(); i++) {
        if (std::isalpha(word[i])) {
            hash_value *= prime_numbers.at(std::tolower(word[i]));
        } else {
            return 0;
        }
    }

    return hash_value;
}

// Common recursive algorithm to find all combinations of list of elements (n!/(r!(n-r)!))
void generateCombinations(const std::string& elements, int k, 
                          std::string& current_combination, int index, 
                          std::unordered_set<unsigned long long>& unique_hashes,
                          const std::unordered_map<char, int>& prime_numbers) {

    if (k == 0) {
        unsigned long long hash = letterHash(current_combination, prime_numbers);
        unique_hashes.insert(hash);
        return;
    }

    if (index == elements.size()) {
        return;
    }

    current_combination += elements[index];
    generateCombinations(elements, k - 1, current_combination, index + 1, unique_hashes, prime_numbers);

    current_combination.pop_back();
    generateCombinations(elements, k, current_combination, index + 1, unique_hashes, prime_numbers);
}

void loadHashmapThread(const std::string& filepath,
                        std::unordered_map<unsigned long long, std::vector<std::string>>& map,
                        const std::unordered_map<char, int>& prime_numbers,
                        int start_line, int end_line) {

    FILE* file = fopen(filepath.c_str(), "r");
    if (!file) {
        std::cerr << "Error opening file: " << filepath << std::endl;
        return;
    }

    // Move to the start position for this thread
    for (int i = 1; i < start_line; ++i) {
        if (fscanf(file, "%*[^\n]\n") == EOF) {
            break;  // Reached end of file
        }
    }

    char buffer[15]; // Adjust the size based on the expected maximum line length
    for (int i = start_line; i <= end_line; ++i) {
        if (fscanf(file, "%15[^\n]\n", buffer) == EOF) {
            break;  // Reached end of file
        }
        std::string line(buffer);
        unsigned long long hash = letterHash(line, prime_numbers);

        map[hash].push_back(line);
    }

    fclose(file);

    return;
}

struct CompareLength {
    bool operator()(const std::string& a, const std::string& b) const {
        if (a.length() == b.length()) {
            return a > b;  // Alphabetical order if lengths are equal
        }
        return a.length() < b.length();  // Longer strings come first
    }
};

int main() {

    const std::unordered_map<char, int> prime_numbers = {
        {'a', 2}, {'b', 3}, {'c', 5}, {'d', 7}, {'e', 11}, {'f', 13},
        {'g', 17}, {'h', 19}, {'i', 23}, {'j', 29}, {'k', 31}, {'l', 37},
        {'m', 41}, {'n', 43}, {'o', 47}, {'p', 53}, {'q', 59}, {'r', 61},
        {'s', 67}, {'t', 71}, {'u', 73}, {'v', 79}, {'w', 83}, {'x', 89},
        {'y', 97}, {'z', 101}
    };

    const int num_threads = 7;
    int file_length;
    std::string example = "waitlists", current_combination, filepath, word;
    std::unordered_set<unsigned long long> unique_hashes;
    std::vector<std::unordered_map<unsigned long long, std::vector<std::string>>> local_hash_maps(num_threads);
    std::priority_queue<std::string, std::vector<std::string>, CompareLength> solutions;
    std::vector<std::thread> threads;

    filepath = "Game-Banks/" + std::to_string(example.length()) + "-game.txt";
    std::ifstream file(filepath);
    file >> file_length;
    file.close();

    auto start_loading = std::chrono::high_resolution_clock::now();

    for (int thread_id = 0; thread_id < num_threads; ++thread_id) {
        int lines_per_thread = file_length / num_threads;
        int start_line = thread_id * lines_per_thread + 1;
        int end_line = (thread_id == num_threads - 1) ? file_length + 1 : (thread_id + 1) * lines_per_thread;

        threads.emplace_back(loadHashmapThread, filepath, std::ref(local_hash_maps[thread_id]), std::ref(prime_numbers), start_line, end_line);
    }

    for (int i = 0; i < num_threads; i++) {
        threads[i].join();
    }

    auto end_loading = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> loading_time = end_loading - start_loading;

    std::cout << "Time taken to load hashmap: " << loading_time.count() << " seconds." << std::endl;


    auto start_lookup = std::chrono::high_resolution_clock::now();

    for (int i = example.length(); i > 2; i--) {
        generateCombinations(example, i, current_combination, 0, unique_hashes, prime_numbers);
    }

    for (int i = 0; i < num_threads; i++) {
        for (auto it = unique_hashes.begin(); it != unique_hashes.end(); ++it) {
            for (const std::string& hashed_word : local_hash_maps[i][*it]) {
                solutions.push(hashed_word);
            }
        }
    }

    auto end_lookup = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> lookup_time = end_lookup - start_lookup;

    std::cout << "Time taken to hash input, find, and sort anagrams: " << lookup_time.count() << " seconds." << std::endl << std::endl;

    std::cout << "Anagrams of word " << example << ":" << std::endl;
    while (!solutions.empty()) {
        std::cout << solutions.top() << std::endl;
        solutions.pop();
    }

    return 0;
}