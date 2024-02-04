#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <chrono>
#include <queue>

unsigned long long letterHash(const std::string& word, const std::unordered_map<char, int>& prime_numbers) {

    unsigned long long hash_value = 1;

    for (char ch : word) {
        if (std::isalpha(ch)) {
            hash_value *= prime_numbers.at(std::tolower(ch));
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

    std::string example = "waitlisted", current_combination, filepath, word;
    std::unordered_map<unsigned long long, std::vector<std::string>> hashed_words;
    std::unordered_set<unsigned long long> unique_hashes;
    std::priority_queue<std::string, std::vector<std::string>, CompareLength> solutions;

    auto start_loading = std::chrono::high_resolution_clock::now();

    filepath = "Game-Banks/" + std::to_string(example.length()) + "-game.txt", word;
    std::ifstream file(filepath);

    while (file >> word) {
        unsigned long long hash = letterHash(word, prime_numbers);
        hashed_words[hash].push_back(word);
    }

    file.close();

    auto end_loading = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> loading_time = end_loading - start_loading;

    std::cout << "Time taken to load hashmap: " << loading_time.count() << " seconds." << std::endl;


    auto start_lookup = std::chrono::high_resolution_clock::now();

    for (int i = example.length(); i > 2; i--) {
        generateCombinations(example, i, current_combination, 0, unique_hashes, prime_numbers);
    }

    for (unsigned long long hash : unique_hashes) {
        for (std::string hashed_word : hashed_words[hash]) {
            solutions.push(hashed_word);
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