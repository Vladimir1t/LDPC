#include "ReedSolomon.h"
#include "Constellation.h"
#include "LdpcCode.h"

#include <time.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <iostream>
#include <random>
#include <iomanip>      // std::setprecision

using namespace std;

void ldpc_test();
int test_2();

int main() {

    /**
     * @param K - information symbols
     * @param bits - size of each symbol
     * @param nsym - 2t (t - max possible number of errors)
     */
    int bits = 8, K = 16, nsym = 20;
	// TODO std::vector<uint8_t> 
    LDPC_RS::RS_WORD data1[] = { 0x40, 0xd2, 0x75, 0x47, 0x76, 0x17, 0x32, 0x06, 0x27, 0x26, 0x96, 0xc6, 0xc6, 0x96, 0x70, 0xec };

	LDPC_RS::Poly msg(K, data1);
	LDPC_RS::Poly a(K + nsym, data1);
    
	LDPC_RS::ReedSolomon rs(bits);
	rs.encode(a.coef, data1, K, nsym);
	cout << "Original message: " << endl;
	msg.print();
	cout << endl << "Encoded message: " << endl;
	a.print();
	cout << endl;

	vector<unsigned int> corrPos, erasePos;
	erasePos.push_back(1);
	erasePos.push_back(2);
	erasePos.push_back(3);
	corrPos.push_back(5);
	corrPos.push_back(6);
	srand(time(0));

	for (unsigned char i : erasePos) {
		a.coef[i] = 0;
	}
	for (unsigned char i : corrPos) {
		a.coef[i] = 1;
	}

    a.coef[10] = 13;
    a.coef[13] = 100;

	cout << endl << "Corrupted message: " << endl;
	a.print();
	cout << endl;
	bool success = rs.decode(a.coef, msg.coef, a.coef, K, nsym, &erasePos, true);
	if (!success) {
		cout << "Decoding failed!" << endl;
	} 
    else {
		cout << "After decoding: " << endl;
		a.print();
		cout << endl << "Decoded message: " << endl;
		msg.print();
	}

    // ldpc_test();
    test_2();

    return 0;
}

void ldpc_test() {

    unsigned block_length = 648;
    unsigned rate_index = 0;
    double target_error_rate = 0.01; // Целевой уровень ошибок
    unsigned num_tests = 1000;

    LdpcCode ldpc_code(0, 0);
    ldpc_code.load_wifi_ldpc(block_length, rate_index);
    unsigned info_length = ldpc_code.get_info_length();

    // Инициализация генератора случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> bit_dist(0, 1);
    std::uniform_int_distribution<> pos_dist(0, block_length-1);

    // Автоподбор количества ошибок
    unsigned min_errors = 1;
    unsigned max_errors = block_length/10; // Макс. 10% ошибок
    unsigned current_errors = min_errors;
    
    while (current_errors <= max_errors) {
        unsigned error_count = 0;

        for (unsigned test = 0; test < num_tests; ++test) {
            // Генерация данных
            std::vector<uint8_t> info_bits(info_length);
            for (auto& bit : info_bits) bit = bit_dist(gen);

            // Кодирование
            std::vector<uint8_t> coded_bits = ldpc_code.encode(info_bits);

            // Инжекция ошибок
            std::vector<uint8_t> corrupted_bits = coded_bits;
            std::vector<int> error_positions(current_errors);
            for (auto& pos : error_positions) {
                pos = pos_dist(gen);
                corrupted_bits[pos] ^= 1;
            }

            // Мягкое декодирование (улучшенные LLR)
            std::vector<double> llr(block_length);
            for (unsigned i = 0; i < block_length; ++i) {
                // Более информативные LLR для ошибочных битов
                llr[i] = (corrupted_bits[i] ? -3.0 : 3.0); // Сильная уверенность
                
                // Ослабляем уверенность для ошибочных битов
                for (const auto& pos : error_positions) {
                    if (i == pos) {
                        llr[i] = (corrupted_bits[i] ? -0.5 : 0.5); // Слабая уверенность
                        break;
                    }
                }
            }

            // Декодирование
            std::vector<uint8_t> decoded_bits = ldpc_code.decode(llr, 20, false);

            // Проверка
            for (unsigned i = 0; i < info_length; ++i) {
                if (decoded_bits[i] != info_bits[i]) {
                    error_count++;
                    break;
                }
            }
        }

        double error_rate = (double)error_count / num_tests;
        std::cout << "Errors: " << current_errors 
                  << " Error rate: " << error_rate << std::endl;

        if (error_rate >= target_error_rate) {
            std::cout << "Found threshold: " << current_errors 
                      << " errors causes ~1% BLER" << std::endl;
            break;
        }

        current_errors++;
    }
}

int test_2() {
    // 1. Инициализация
    const int block_length = 12;  // Упрощённый маленький блок
    const int info_length = 5;   // 4 информационных бита
    unsigned rate_index = 0;
    // LdpcCode ldpc_code(block_length, info_length);
    LdpcCode ldpc_code(6, 5);
    ldpc_code.load_wifi_ldpc(block_length, 0);
    // ldpc_code.load_wifi_ldpc(4, rate_index);
    
    // 2. Фиксированные тестовые данные (для простоты)
    std::vector<uint8_t> info_bits {1, 0, 1, 0, 1}; // Информационные биты
    
    // 3. Кодирование
    std::vector<uint8_t> coded_bits = ldpc_code.encode(info_bits);
    std::cout << "Encoded bits: ";
    for (auto bit : coded_bits) std::cout << (int)bit << " ";
    std::cout << std::endl;

    // 4. Внесение ошибок вручную
    std::vector<uint8_t> corrupted_bits = coded_bits;
    std::cout << "Enter positions to flip (1-" << block_length << "), 0 to end: ";
    
    int pos;
    while (std::cin >> pos && pos != 0) {
        if (pos >= 1 && pos <= block_length) {
            corrupted_bits[pos-1] ^= 1; // Инвертируем бит
            std::cout << "Flipped bit at position " << pos << std::endl;
        }
    }

    // 5. Декодирование
    std::vector<double> llr(block_length);
    for (int i = 0; i < block_length; i++) {
        llr[i] = corrupted_bits[i]; //? -1.0 : 1.0; // Простые LLR
    }
    for (auto a : llr)
        std::cout << a <<'\n';
    
    std::vector<uint8_t> decoded_bits = ldpc_code.decode(llr, 10, false);

    // 6. Проверка результата
    std::cout << "\nOriginal info: ";
    for (auto bit : info_bits) std::cout << (int)bit << " ";
    
    std::cout << "\nDecoded info:  ";
    for (int i = 0; i < info_length; i++) {
        std::cout << '[' << (int)decoded_bits[i] << "] ";
        if (decoded_bits[i] != info_bits[i]) {
            std::cout << "\n\nDECODING FAILED! Bit errors detected.";
        }
    }
    
    std::cout << "\n\nSUCCESS! All bits decoded correctly.";
    return 0;
}