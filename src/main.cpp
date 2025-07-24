#include "ReedSolomon.h"
#include "Constellation.h"
#include "LdpcCode.h"

#include <time.h>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <iostream>
#include <random>


int test_RE_LDPC_LSR_level();
int test_ldpc();
std::vector<uint8_t> bytes_to_bits(const std::vector<uint8_t>& bytes);
std::vector<uint8_t> bits_to_bytes(const std::vector<uint8_t>& bits);
std::vector<uint8_t> ldpc_encode(std::vector<uint8_t>& info_bits, size_t info_size, int& block_length, unsigned& rate_index);
std::vector<uint8_t> ldpc_decode(std::vector<double>& llr, size_t info_size, int block_length, unsigned rate_index);
void add_noise(std::vector<double>& data, double error_prob);
void noise_resistance_test();


int main() {

    // 2 Tests for checking LDPC + RS algorithm

    test_RE_LDPC_LSR_level();

    noise_resistance_test();
}

int test_RE_LDPC_LSR_level() {

    /**
     * @param K - information symbols
     * @param bits - size of each symbol
     * @param nsym - 2t (t - max possible number of errors)
     */
	// TODO std::vector<uint8_t> 
    std::vector<LDPC_RS::RS_WORD> data1 = { 0x40, 0xd2, 0x75, 0x47, 0x76, 0x17, 0x32, 0x06, 0x27, 0x26, 0x96, 0xc6, 0xc6, 0x96, 0x70, 0xec, 100, 403, 13043, 2123};
    int bits = 8;
    int K = data1.size();
    int nsym = K * 3 / 2;

    //-----------------------------------------
	LDPC_RS::Poly msg(K, data1.data());
	LDPC_RS::Poly poly_a(K + nsym, data1.data());
    
	LDPC_RS::ReedSolomon rs(bits);
	rs.encode(poly_a.coef, data1.data(), K, nsym);
    #ifdef DEBUG
	cout << "Original message: " << endl;
	msg.print();

	cout << endl << "Encoded message: " << endl;
	poly_a.print();
	cout << endl;
    #endif

    //-----------------------------------------
	std::vector<unsigned> corrPos, erasePos;
    #if 0
	erasePos.push_back(1);
	erasePos.push_back(2);
	erasePos.push_back(3);
	corrPos.push_back(5);
	corrPos.push_back(6);
	srand(time(0));

	for (unsigned char i : erasePos) {
		poly_a.coef[i] = 0;
	}
	for (unsigned char i : corrPos) {
		poly_a.coef[i] = 1;
	}
    poly_a.coef[10] = 13;
    poly_a.coef[13] = 100;

	std::cout << endl << "Corrupted message: " << endl;
	poly_a.print();
	cout << endl;
    #endif
    //-----------------------------------------

    std::vector<uint8_t> encoded = poly_a.get_vector();

    int block_length = 648; 
    unsigned rate_index = 0; 
    unsigned n_bits =  1;    // (1=BPSK, 2=4-ASK, 3=8-ASK)

    std::vector<uint8_t> bits_for_ldpc = bytes_to_bits(encoded);

    unsigned origin_size = bits_for_ldpc.size();

    std::vector<uint8_t> ldpc_encoded = ldpc_encode(bits_for_ldpc, bits_for_ldpc.size(), block_length, rate_index);

    //-----------------------------------------
    // ldpc_encoded[1] = 1;
    // ldpc_encoded[6] = 1;
    // ldpc_encoded[10] = 1;
    // ldpc_encoded[15] = 1;
    // ldpc_encoded[200] = 1;
    // ldpc_encoded[25] = 1;
    // ldpc_encoded[100] = 1;
    // ldpc_encoded[150] = 1;
    //-----------------------------------------

    // std::cout << "---- Modulate LDPC ----\n"; 

    Constellation modulation(n_bits);

    std::vector<double> mod_sym = modulation.modulate(ldpc_encoded);

    #if 1 // Nois 
    for (double ebno_db = 1.2; ebno_db > 0.85; ebno_db -= 0.01) {
        
        std::vector<double> mod_sym_it = mod_sym;
        double snr_linear = std::pow(10.0, ebno_db / 10) * (block_length/2) / block_length * n_bits;
        double noise_var = 0.5 / snr_linear; 

        std::vector<double> received_signal(mod_sym_it.size());
        std::default_random_engine generator;
        std::normal_distribution<double> noise_dist(0.0, sqrt(noise_var));

        for (size_t i = 0; i < mod_sym_it.size(); i++) {
            received_signal[i] = mod_sym_it[i] + noise_dist(generator);
        }
        std::vector<double> llr = modulation.llr_compute(received_signal, noise_var);
        #endif

        // std::cout << "---- Demodulate LDPC ----\n"; 

        #if 0 // without nois
        std::vector<double> llr = modulation.llr_compute(mod_sym_it, 1);
        #endif
        //-----------------------------------------

        std::vector<uint8_t> ldpc_decoded = ldpc_decode(llr, origin_size, block_length, rate_index);

        std::vector<uint8_t> bytes_for_rs = bits_to_bytes(ldpc_decoded);

        std::vector<LDPC_RS::RS_WORD> ldpc_decoded_rs;
        ldpc_decoded_rs.reserve(bytes_for_rs.size());
        for (auto coef : bytes_for_rs)
            ldpc_decoded_rs.push_back(coef);

        //-----------------------------------------

        #ifdef DEBUG
        std::cout << "Original info:  ";
        for (size_t i = 0; i < encoded.size(); i++) 
            std::cout << (int)encoded[i] << " ";
        std::cout << "\n";

        std::cout << "Decoded info:   ";
        for (size_t i = 0; i < bytes_for_rs.size(); i++)
            std::cout << (int)bytes_for_rs[i] << " ";
        std::cout << "\n";

        bool error = false;
        for (size_t i = 0; i < encoded.size(); i++) {
            if (encoded[i] != bytes_for_rs[i]) {
                error = true;
                break;
            }
        }

        if (error) {
            std::cout << "\nDECODING FAILED LDPC! Bit errors detected.\n"
                      << "ebno_db = " << ebno_db << "\n\n";;
        } else {
            std::cout << "\nLDPC SUCCESS! All bits decoded correctly.\n"
                      << "ebno_db = " << ebno_db << "\n\n";
        }
        #endif

        //-----------------------------------------

	    bool success = rs.decode(ldpc_decoded_rs.data(), msg.coef, ldpc_decoded_rs.data(), K, nsym, &erasePos, true);
	    if (!success) {
            std::cout << "-------------------------------------------------------------------\n";
	    	std::cout << "Decoding RS + LDPC  \033[31m[ FAILED ]\033[0m\n"
                      << "NOIS LEVEL: ebno_db = \033[33m" << ebno_db << "\033[0m\n";
            std::cout << "-------------------------------------------------------------------\n\n";
	    } 
        else {
            std::cout << "-------------------------------------------------------------------\n\n";
            std::cout << "Decoding RS + LDPC  \033[32;40m[ SUCCESS ]\033[0m\n" 
                      << "NOIS LEVEL: ebno_db = \033[33m" << ebno_db << "\033[0m\n\n";
            #ifdef DEBUG
	    	cout << endl << "Decoded message: " << endl;
	    	msg.print();
            #endif
            std::cout << "-------------------------------------------------------------------\n\n";
	    }
    }

    return 0;
}

std::vector<uint8_t> bytes_to_bits(const std::vector<uint8_t>& bytes) {
    std::vector<uint8_t> bits;
    bits.reserve(bytes.size() * 8);
    
    for (uint8_t byte : bytes) {
        for (int i = 0; i < 8; ++i) {
            bits.push_back((byte >> (7-i)) & 1);
        }
    }
    
    return bits;
}

std::vector<uint8_t> bits_to_bytes(const std::vector<uint8_t>& bits) {
    std::vector<uint8_t> bytes;
    bytes.reserve((bits.size() + 7) / 8);
    
    for (size_t i = 0; i < bits.size(); i += 8) {
        uint8_t byte = 0;
        for (int j = 0; j < 8 && (i+j) < bits.size(); ++j) {
            byte |= (bits[i+j] << (7-j));
        }
        bytes.push_back(byte);
    }
    
    return bytes;
}

void add_noise(std::vector<double>& data, double error_prob) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (size_t i = 0; i < data.size(); i++) {
        if (dis(gen) < error_prob) {
            data[i] *= -1;  
        }
    }
}

void noise_resistance_test() {
   
    std::vector<LDPC_RS::RS_WORD> original_data = {0x40, 0xD2, 0x75, 0x47, 0x76, 0x17, 0x32, 0x06, 100, 14, 89, 0x32, 0x06, 0x27, 0x26, 0x96, 0xc6, 0xc6, 0x96, 0x70, 0xec, 100, 403, 13043, 2123};

    int bits = 8;
    int K = original_data.size();
    int nsym = K * 3 / 2;

    //-----------------------------------------
	LDPC_RS::Poly msg(K, original_data.data());
	LDPC_RS::Poly poly_a(K + nsym, original_data.data());
    
	LDPC_RS::ReedSolomon rs(bits);
	rs.encode(poly_a.coef, original_data.data(), K, nsym);

    #ifdef DEBUG
	cout << "Original message: " << endl;
	msg.print();

	cout << endl << "Encoded message: " << endl;
	poly_a.print();
	cout << endl;
    #endif

    //-----------------------------------------
	std::vector<unsigned> corrPos, erasePos;

   std::vector<uint8_t> encoded = poly_a.get_vector();

    int block_length = 648; 
    unsigned rate_index = 0; 
    unsigned n_bits =  1;    // (1=BPSK, 2=4-ASK, 3=8-ASK)

    std::vector<uint8_t> bits_for_ldpc = bytes_to_bits(encoded);

    unsigned origin_size = bits_for_ldpc.size();

    std::vector<uint8_t> ldpc_encoded = ldpc_encode(bits_for_ldpc, bits_for_ldpc.size(), block_length, rate_index);


    // Probabiliry of error 0% - 10%
    for (double error_prob = 0.0; error_prob <= 0.15; error_prob += 0.01) {
        
        std::vector<uint8_t> noisy_data = ldpc_encoded;

        Constellation modulation(n_bits);
        std::vector<double> mod_sym = modulation.modulate(noisy_data);

        // Add noise
        add_noise(mod_sym, error_prob);
    
        std::vector<double> llr = modulation.llr_compute(mod_sym, 1);

        //-----------------------------------------

        std::vector<uint8_t> ldpc_decoded = ldpc_decode(llr, origin_size, block_length, rate_index);

        std::vector<uint8_t> bytes_for_rs = bits_to_bytes(ldpc_decoded);

        std::vector<LDPC_RS::RS_WORD> ldpc_decoded_rs;
        ldpc_decoded_rs.reserve(bytes_for_rs.size());
        for (auto coef : bytes_for_rs)
            ldpc_decoded_rs.push_back(coef);
        bool success = rs.decode(ldpc_decoded_rs.data(), msg.coef, ldpc_decoded_rs.data(), K, nsym, &erasePos, true);

        // int errors = 0;
        // for (size_t i = 0; i < original_data.size(); i++) {
        //     if (original_data[i] != msg.coef[i]) {
        //         errors++;
        //     }
        // }

        // Results
        std::cout << "-------------------------------------------------------------------\n";
        std::cout << "Error probability: \033[33m" << error_prob * 100 << "%\033[0m | ";
        std::cout << "Errors after decoding: " << ((success) ? "\033[32;40m[ SUCCESS ]\033[0m 0" : "\033[31m[ FAILED ]\033[0m") << "/" << original_data.size() << "\n";
        std::cout << "-------------------------------------------------------------------\n\n";
    }
}


std::vector<uint8_t> ldpc_encode(std::vector<uint8_t>& info_bits, size_t info_size, int& block_length, unsigned& rate_index) {

   // std::cout << "---- Encode LDPC ----\n"; 

    LDPC_RS::LdpcCode ldpc_code(0, 0);

    #if 1
        if (info_size < 648 / 2)
            rate_index = 0;
        else if (info_size < 1296 / 2)
            block_length = 1296;
        else if (info_size < 1944 / 2)
            block_length = 1944;

        ldpc_code.load_wifi_ldpc(block_length, rate_index);
        info_bits.resize(ldpc_code.get_info_length());
    #endif

    #if 0
    unsigned info_length = ldpc_code.get_info_length();

    std::vector<uint8_t> info_bits(info_length);
    for (auto& bit : info_bits) bit = rand() % 2;
    #endif

    //std::cout << "encode [2]\n"; 
   
    std::vector<uint8_t> coded_bits = ldpc_code.encode(info_bits);

    //std::cout << "encode [3]\n"; 

    return coded_bits;
}

std::vector<uint8_t> ldpc_decode(std::vector<double>& llr, size_t info_size, int block_length, unsigned rate_index) {

    // std::cout << "---- Decode LDPC-----\n"; 
    LDPC_RS::LdpcCode ldpc_code(0, 0);

    ldpc_code.load_wifi_ldpc(block_length, rate_index);

    std::vector<uint8_t> decoded_bits = ldpc_code.decode(llr, 20, true); 
    decoded_bits.resize(info_size);

    return decoded_bits;
}

int test_ldpc() {
   
    std::cout << "\n---- LDPC TEST ----\n\n";
   
    int block_length = 648; 
    unsigned rate_index = 0; 
    unsigned n_bits = 1;    // (1=BPSK, 2=4-ASK, 3=8-ASK)
    double ebno_db = 5.0;   // Отношение сигнал/шум в dB

    LDPC_RS::LdpcCode ldpc_code(0, 0);

    #if 1
        std::vector<uint8_t> info_bits = {1, 1, 0, 1, 0, 0, 1, 1, 0, 1};
        size_t info_size = info_bits.size();

        if (info_size < 648 / 2)
            rate_index = 0;
        else if (info_size < 1296 / 2)
            block_length = 1296;
        else if (info_size < 1944 / 2)
            block_length = 1944;

        ldpc_code.load_wifi_ldpc(block_length, rate_index);
        info_bits.resize(ldpc_code.get_info_length());
    #endif

    #if 0
    unsigned info_length = ldpc_code.get_info_length();

    std::vector<uint8_t> info_bits(info_length);
    for (auto& bit : info_bits) bit = rand() % 2;
    #endif

   
    std::vector<uint8_t> coded_bits = ldpc_code.encode(info_bits);

    // Modulation
    Constellation modulation(n_bits);
   
    std::vector<double> mod_sym = modulation.modulate(coded_bits);

    // Noise
    double snr_linear = std::pow(10.0, ebno_db / 10) * ldpc_code.get_info_length() / block_length * n_bits;
    double noise_var = 0.5 / snr_linear; 
    
    std::vector<double> received_signal(mod_sym.size());
    std::default_random_engine generator;
    std::normal_distribution<double> noise_dist(0.0, sqrt(noise_var));
    
    for (size_t i = 0; i < mod_sym.size(); i++) {
        received_signal[i] = mod_sym[i] + noise_dist(generator);
    }

    // (LLR)
    std::vector<double> llr = modulation.llr_compute(received_signal, noise_var);

    std::vector<uint8_t> decoded_bits = ldpc_code.decode(llr, 20, true); 

    bool error = false;
    for (size_t i = 0; i < info_size; i++) {
        if (decoded_bits[i] != info_bits[i]) {
            error = true;
            break;
        }
    }
    std::cout << "Original info:  ";
    for (size_t i = 0; i < info_size; i++) 
        std::cout << (int)info_bits[i] << " ";
    std::cout << "\n";

    std::cout << "Decoded info:   ";
    for (size_t i = 0; i < info_size; i++)
        std::cout << (int)decoded_bits[i] << " ";
    std::cout << "\n";

    if (error) {
        std::cout << "\nDECODING FAILED! Bit errors detected.\n";
    } else {
        std::cout << "\nSUCCESS! All bits decoded correctly.\n";
    }

    return 0;
}



// void ldpc_test() {

//     unsigned block_length = 648;
//     unsigned rate_index = 0;
//     double target_error_rate = 0.01; // Целевой уровень ошибок
//     unsigned num_tests = 1000;

//     LDPC_RS::LdpcCode ldpc_code(0, 0);
//     ldpc_code.load_wifi_ldpc(block_length, rate_index);
//     unsigned info_length = ldpc_code.get_info_length();

//     // Инициализация генератора случайных чисел
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<> bit_dist(0, 1);
//     std::uniform_int_distribution<> pos_dist(0, block_length-1);

//     // Автоподбор количества ошибок
//     unsigned min_errors = 1;
//     unsigned max_errors = block_length/10; // Макс. 10% ошибок
//     unsigned current_errors = min_errors;
    
//     while (current_errors <= max_errors) {
//         unsigned error_count = 0;

//         for (unsigned test = 0; test < num_tests; ++test) {
//             // Генерация данных
//             std::vector<uint8_t> info_bits(info_length);
//             for (auto& bit : info_bits) bit = bit_dist(gen);

//             // Кодирование
//             std::vector<uint8_t> coded_bits = ldpc_code.encode(info_bits);

//             // Инжекция ошибок
//             std::vector<uint8_t> corrupted_bits = coded_bits;
//             std::vector<int> error_positions(current_errors);
//             for (auto& pos : error_positions) {
//                 pos = pos_dist(gen);
//                 corrupted_bits[pos] ^= 1;
//             }

//             // Мягкое декодирование (улучшенные LLR)
//             std::vector<double> llr(block_length);
//             for (unsigned i = 0; i < block_length; ++i) {
//                 // Более информативные LLR для ошибочных битов
//                 llr[i] = (corrupted_bits[i] ? -3.0 : 3.0); // Сильная уверенность
                
//                 // Ослабляем уверенность для ошибочных битов
//                 for (const auto& pos : error_positions) {
//                     if (i == pos) {
//                         llr[i] = (corrupted_bits[i] ? -0.5 : 0.5); // Слабая уверенность
//                         break;
//                     }
//                 }
//             }

           
//             std::vector<uint8_t> decoded_bits = ldpc_code.decode(llr, 20, false);

//             for (unsigned i = 0; i < info_length; ++i) {
//                 if (decoded_bits[i] != info_bits[i]) {
//                     error_count++;
//                     break;
//                 }
//             }
//         }

//         double error_rate = (double)error_count / num_tests;
//         std::cout << "Errors: " << current_errors 
//                   << " Error rate: " << error_rate << std::endl;

//         if (error_rate >= target_error_rate) {
//             std::cout << "Found threshold: " << current_errors 
//                       << " errors causes ~1% BLER" << std::endl;
//             break;
//         }

//         current_errors++;
//     }
// }