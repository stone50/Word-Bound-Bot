#include <iostream>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <exception>

#define MAX_WORD_LENGTH 8

struct Word {
	char str[MAX_WORD_LENGTH];
};

int word_length;
std::unordered_set<char> unsure_letters;
std::unordered_set<char> using_letters;
std::vector<std::unordered_set<char>> letter_possibilities;
char input[26];
std::vector<Word> possible_words;
bool game_over = false;

void setup();
void cleanPossibleWords();
void guess();
void getDetails(char guess[]);
void removeWord(char str[]);

class CannotOpenFile : public std::exception
{
	virtual const char* what() const throw()
	{
		return "Could not open file";
	}
} COF;	//(Could not Open File)

class FileCannotRename : public std::exception
{
	virtual const char* what() const throw()
	{
		return "Could not rename file";
	}
} FCR;	//(File Could not be Renamed)

int main() {
	try {
		setup();

		while (possible_words.size() && !game_over) {
			guess();
		}
	}
	catch (std::exception& e) {
		std::cout << "\nAN ERROR OCCURRED: " << e.what() << std::endl;
	}

	return 0;
}

//gets starting info from user and initializes variables
void setup() {

	//get word length from user
	std::cout << "How many letters are in the word?\n";
	std::cin.getline(input, sizeof(input));
	word_length = strtol(input, NULL, 10);

	//get available letters from user
	std::cout << "\nWhich letters are available?\n";
	std::cin.getline(input, sizeof(input));

	//populate unordered sets
	for (unsigned int letter = 0; letter < strlen(input); letter++) {
		unsure_letters.insert(input[letter]);
	}
	for (int i = 0; i < word_length; i++) {
		std::unordered_set<char> temp;
		for (unsigned int letter = 0; letter < strlen(input); letter++) {
			temp.insert(input[letter]);
		}
		letter_possibilities.push_back(temp);
	}

	//add possible words with the correct length to possible_words
	char file_line[MAX_WORD_LENGTH + 1];
	std::ifstream words_list;
	words_list.open("words.txt");
	if (words_list.is_open()) {
		while (words_list.good()) {
			words_list >> file_line;
			if (strlen(file_line) == word_length) {
				Word new_word;
				strcpy_s(new_word.str, file_line);
				possible_words.push_back(new_word);
			}
		}
		words_list.close();
	}
	else {
		throw COF;
	}

	cleanPossibleWords();
}

//filters the list of possible words
void cleanPossibleWords() {
	std::cout << "\nrefining posibilities...\n";
	for (unsigned int i = 0; i < possible_words.size(); i++) {
		bool delete_word = false;
		char* current_word = possible_words[i].str;
		int used_letters = 0;
		for (unsigned int letter = 0; letter < strlen(current_word); letter++) {
			if (!letter_possibilities[letter].count(current_word[letter])) {
				delete_word = true;
				break;
			}
			if (using_letters.count(current_word[letter])) {
				used_letters++;
			}
		}
		if (!delete_word) {
			for (const auto& letter : using_letters) {
				if (!strchr(current_word, letter)) {
					delete_word = true;
					break;
				}
			}
		}
		if (delete_word) {
			possible_words.erase(possible_words.begin() + i);
			i--;
		}
	}
}

//makes a guess and gets feedback from user
void guess() {
	while (true) {

		//make a guess
		char* guess = possible_words[0].str;
		std::cout << "\nIs the word '" << guess << "'? ('y' = yes | 'n' = no | 'd' = word does not exist)\n";
		std::cin >> input;
		while (input[0] != 'y' && input[0] != 'n' && input[0] != 'd') {
			std::cout << "\nPlease enter 'y' for yes, 'n' for no, or 'd' for word does not exist\n";
			std::cin >> input;
		}

		switch (input[0]) {
		case 'y':
			std::cout << "\nYay, I won!\n";
			//TODO	ask user to play again
			game_over = true;
			return;

		case 'n':
			getDetails(guess);
			cleanPossibleWords();
			if (possible_words.size() == 0) {
				std::cout << "\nI've run out of guesses. I'm stumped!\n";
				//TODO ask user to play again
			}
			return;

		case 'd':
			removeWord(guess);
			break;
		}
	}
	
}

//gets information about individual letters from user
void getDetails(char guess[]) {
	for (unsigned int letter = 0; letter < strlen(guess); letter++) {
		if (letter_possibilities[letter].size() > 1) {
			if (!using_letters.count(guess[letter]) && unsure_letters.count(guess[letter])) {

				//ask if letter is in the word
				std::cout << "\nIs the letter '" << guess[letter] << "' in the word? ('y' = yes | 'n' = no)\n";
				std::cin >> input;
				while (input[0] != 'y' && input[0] != 'n') {
					std::cout << "\nPlease enter 'y' for yes or 'n' for no\n";
					std::cin >> input;
				}
				switch (input[0]) {
				case 'y':
					using_letters.insert(guess[letter]);
					break;

				case 'n':
					for (unsigned int i = 0; i < letter_possibilities.size(); i++) {
						letter_possibilities[i].erase(guess[letter]);

						//check if only one letter is possible for a given position. If true, that letter must be added to using_letters
						if (letter_possibilities[i].size() == 1) {
							using_letters.insert(*letter_possibilities[i].begin());
						}
					}
					break;
				}
				unsure_letters.erase(guess[letter]);
			}
			if (using_letters.count(guess[letter])) {

				//ask if letter is in current position
				std::cout << "\nIs the letter '" << guess[letter] << "' in position " << letter + 1 << "? ('y' = yes | 'n' = no)\n";
				std::cin >> input;
				while (input[0] != 'y' && input[0] != 'n') {
					std::cout << "\nPlease enter 'y' for yes or 'n' for no\n";
					std::cin >> input;
				}
				switch (input[0]) {
				case 'y':
					letter_possibilities[letter].clear();
					letter_possibilities[letter].insert(guess[letter]);
					break;

				case 'n':
					letter_possibilities[letter].erase(guess[letter]);
					break;
				}
			}
		}
	}
}

//removes the given word from possible_words and words.txt
void removeWord(char str[]) {
	std::cout << "\nremoving '" << str << "' from dictionary...\n";

	//remove word from words.txt
	char file_line[MAX_WORD_LENGTH + 1];
	std::ifstream old_words_list;
	std::ofstream new_words_list;
	old_words_list.open("words.txt");
	new_words_list.open("new_words.txt");
	if (old_words_list.is_open() && new_words_list.is_open()) {
		bool begin = true;
		while (old_words_list.good() && new_words_list.good()) {
			old_words_list >> file_line;
			if (strcmp(file_line, str)) {
				if (!begin) {
					new_words_list << "\n";
				}
				else {
					begin = false;
				}
				new_words_list << file_line;
			}
		}
		old_words_list.close();
		new_words_list.close();
		remove("words.txt");
		if (rename("new_words.txt", "words.txt")) {
			throw FCR;
		}
	}
	else {
		throw COF;
	}

	//remove word from possible_words
	for (unsigned int i = 0; i < possible_words.size(); i++) {
		if (!strcmp(possible_words[i].str, str)) {
			possible_words.erase(possible_words.begin() + i);
			break;
		}
	}
}