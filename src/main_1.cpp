// #include <iostream>
// #include <memory>
// #include <vector>
// #include <string>

// #include <aff3ct.hpp>
// // #include "/Users/a/Desktop/LDPC/aff3ct/include/Module/Decoder/RS/Decoder_RS.hpp"

// using namespace aff3ct;

// struct params
// {
//     const int m = 7;       // GF(2^8) - 8 bits per symbol
//     const int K = 31;     // Information symbols
//     const int N = 127;     // Codeword symbols (2^m - 1)
    
//     // Derived bit-level parameters
//     const int K_rs = K * m;   // 223 * 8 = 1784 bits
//     const int N_rs = N * m;   // 255 * 8 = 2040 bits
//     const int n_rdncy = (N - K) / 2; // 32 redundancy symbols

// 	int   fe        =  100;     // number of frame errors
// 	int   seed      =  0;       // PRNG seed for the AWGN channel
// 	float ebn0_min  =  0.00f;   // minimum SNR value
// 	float ebn0_max  =  10.01f;  // maximum SNR value
// 	float ebn0_step =  1.00f;   // SNR step
// 	float R;                    // code rate (R=K/N)
// };

// struct modules
// {
// 	std::unique_ptr<spu::module::Source_random<>>     source;
// 	std::unique_ptr<module::Encoder_RS<>>             encoder;
// 	std::unique_ptr<module::Modem_BPSK<>>             modem;
// 	std::unique_ptr<module::Channel_AWGN_LLR<>>       channel;
// 	std::unique_ptr<module::Decoder_RS<>>             decoder;
// 	std::unique_ptr<module::Monitor_BFER<>>           monitor;
// };

// struct buffers
// {
// 	std::vector<int>     ref_bits;
// 	std::vector<int>     enc_bits;
// 	std::vector<float>   symbols;
// 	std::vector<float>   sigma;
// 	std::vector<float>   noisy_symbols;
// 	std::vector<float>   LLRs;
// 	std::vector<int>     dec_bits;
// };

// struct utils
// {
// 	std::unique_ptr<tools::Sigma<>>                    noise;     // a sigma noise type
// 	std::vector<std::unique_ptr<spu::tools::Reporter>> reporters; // list of reporters dispayed in the terminal
// 	std::unique_ptr<spu::tools::Terminal_std>          terminal;  // manage the output text in the terminal
// };

// void init_utils(const modules &m, utils &u);
// void init_modules( params &p, modules &m);
// void init_params(params &p);
// void init_buffers(const params &p, buffers &b);

// int main(int argc, char** argv)
// {
// 	const std::string v = "v" + std::to_string(tools::version_major()) + "." +
// 	                            std::to_string(tools::version_minor()) + "." +
// 	                            std::to_string(tools::version_release());

// 	std::cout << "#----------------------------------------------------------"      << std::endl;
// 	std::cout << "# This is a basic program using the AFF3CT library (" << v << ")" << std::endl;
// 	std::cout << "# Feel free to improve it as you want to fit your needs."         << std::endl;
// 	std::cout << "#----------------------------------------------------------"      << std::endl;
// 	std::cout << "#"                                                                << std::endl;

// 	params p;  init_params (p   );  // create and initialize the parameters defined by the user
// 	modules m; init_modules(p, m);  // create and initialize the modules
// 	buffers b; init_buffers(p, b);  // create and initialize the buffers required by the modules
// 	utils u;   init_utils  (m, u);  // create and initialize the utils

// 	// display the legend in the terminal
// 	u.terminal->legend();

// 	// loop over the various SNRs
// 	for (auto ebn0 = p.ebn0_min; ebn0 < p.ebn0_max; ebn0 += p.ebn0_step)
// 	{
// 		// compute the current sigma for the channel noise
// 		const auto esn0 = tools::ebn0_to_esn0(ebn0, p.R);
// 		std::fill(b.sigma.begin(), b.sigma.end(), tools::esn0_to_sigma(esn0));

// 		u.noise->set_values(b.sigma[0], ebn0, esn0);

// 		// display the performance (BER and FER) in real time (in a separate thread)
// 		u.terminal->start_temp_report();

// 		// run the simulation chain
// 		while (!m.monitor->fe_limit_achieved() /*&& !u.terminal->is_interrupt()*/)
// 		{
//             std::cout << "[0]\n";
// 			m.source ->generate    (                          b.ref_bits   );
//             std::cout << "[generate] " << b.ref_bits.size() << ' ' << b.enc_bits.size() << '\n';
//             //b.ref_bits.resize(p.K * 7, 1);
//             //b.enc_bits.resize(p.N * 7, 1);

// 			m.encoder->encode      (         b.ref_bits,      b.enc_bits     );
//             std::cout << "[encode]\n";

// 		    m.modem  ->modulate    (         b.enc_bits,      b.symbols      );
//             std::cout << "[modulate]\n";

// 			m.channel->add_noise   (b.sigma, b.symbols,       b.noisy_symbols);
//             std::cout << "[add_noise]]\n";

// 			m.modem  ->demodulate  (b.sigma, b.noisy_symbols, b.LLRs         );
//             std::cout << "[demodulate]\n";

// 			m.decoder->decode_siho (         b.LLRs,          b.dec_bits     );
//             std::cout << "[decode_siho]\n";

// 			m.monitor->check_errors(         b.dec_bits,      b.ref_bits     );
//             std::cout << "[check_errors]\n";
// 		}

// 		// display the performance (BER and FER) in the terminal
// 		u.terminal->final_report();

// 		// reset the monitor for the next SNR
// 		m.monitor->reset();
// 		// u.terminal->reset();

// 		// if user pressed Ctrl+c twice, exit the SNRs loop
// 		// if (u.terminal->is_over()) break;
// 	}

// 	std::cout << "# End of the simulation" << std::endl;

// 	return 0;
// }

// void init_params(params &p)
// {
// 	p.R = (float)p.K / (float)p.N;
// 	std::cout << "# * Simulation parameters: "              << std::endl;
// 	std::cout << "#    ** Frame errors   = " << p.fe        << std::endl;
// 	std::cout << "#    ** Noise seed     = " << p.seed      << std::endl;
// 	std::cout << "#    ** Info. bits (K) = " << p.K         << std::endl;
// 	std::cout << "#    ** Frame size (N) = " << p.N         << std::endl;
// 	std::cout << "#    ** Code rate  (R) = " << p.R         << std::endl;
// 	std::cout << "#    ** SNR min   (dB) = " << p.ebn0_min  << std::endl;
// 	std::cout << "#    ** SNR max   (dB) = " << p.ebn0_max  << std::endl;
// 	std::cout << "#    ** SNR step  (dB) = " << p.ebn0_step << std::endl;
// 	std::cout << "#"                                        << std::endl;
// }

// void init_modules( params &p, modules &m)
// {
//     // const int m_gf = 8; // Размерность поля GF(2^8)
//     // const int N_rs = p.N / m_gf; // Длина кода в символах (должна быть <= 255)
//     // const int K_rs = p.K / m_gf; // Информационных символов

//     // if (p.K % m_gf != 0 || p.N % m_gf != 0) {
//     //     throw std::runtime_error("K and N must be divisible by m (symbol size)");
//     // }

//     // // Create the Galois Field generator first
//     tools::RS_polynomial_generator GF(p.N, (p.N - p.K)/2 * 7); // You'll need to define m and n_rdncy in your params

// 	m.source  = std::unique_ptr<spu::module::Source_random    <>>(new spu::module::Source_random<>(p.K * 7 * 7));
//     std::cout << "[Source_random]\n";

// 	m.encoder = std::unique_ptr<module::Encoder_RS            <>>(new module::Encoder_RS<>(p.K * 7, p.N * 7, GF));
//     std::cout << "[Encoder_RS]\n";

// 	m.modem   = std::unique_ptr<module::Modem_BPSK            <>>(new module::Modem_BPSK<>(p.N_rs ));
//     std::cout << "[Modem_BPSK]\n";

// 	m.channel = std::unique_ptr<module::Channel_AWGN_LLR      <>>(new module::Channel_AWGN_LLR<>(p.N ));
//     std::cout << "[Channel_AWGN_LLR]\n";

//     m.decoder = std::unique_ptr<module::Decoder_RS_std        <>>(new module::Decoder_RS_std<>(p.K * 7, p.N * 7, GF));	
//     std::cout << "[Decoder_RS_std]\n";

//     m.monitor = std::unique_ptr<module::Monitor_BFER          <>>(new module::Monitor_BFER<>(p.K, p.fe));
//     std::cout << "[Monitor_BFER]\n";
// 	m.channel->set_seed(p.seed);
// };

// void init_buffers(const params &p, buffers &b)
// {
// 	b.ref_bits      = std::vector<int  >(p.K * 7 * 7);
// 	b.enc_bits      = std::vector<int  >(p.N * 7 * 7);
// 	b.symbols       = std::vector<float>(p.N);
// 	b.sigma         = std::vector<float>(  1);
// 	b.noisy_symbols = std::vector<float>(p.N);
// 	b.LLRs          = std::vector<float>(p.N);
// 	b.dec_bits      = std::vector<int  >(p.K);
// }

// void init_utils(const modules &m, utils &u)
// {
// 	// create a sigma noise type
// 	u.noise = std::unique_ptr<tools::Sigma<>>(new tools::Sigma<>());
// 	// report the noise values (Es/N0 and Eb/N0)
// 	u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new tools::Reporter_noise<>(*u.noise)));
// 	// report the bit/frame error rates
// 	u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new tools::Reporter_BFER<>(*m.monitor)));
// 	// report the simulation throughputs
// 	u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new tools::Reporter_throughput<>(*m.monitor)));
// 	// create a terminal that will display the collected data from the reporters
// 	u.terminal = std::unique_ptr<spu::tools::Terminal_std>(new spu::tools::Terminal_std(u.reporters));
// }






#include <iostream>
#include <memory>
#include <vector>
#include <string>

#include <aff3ct.hpp>
using namespace aff3ct;

struct params
{
	int   K         =  57;     // number of information bits
	int   N         = 127;     // codeword size
    const int m = 7;          
    const int t = 35;
    const int n_rdncy = t * m; 

	int   fe        = 100;     // number of frame errors
	int   seed      =   0;     // PRNG seed for the AWGN channel
	float ebn0_min  =   0.00f; // minimum SNR value
	float ebn0_max  =  10.01f; // maximum SNR value
	float ebn0_step =   1.00f; // SNR step
	float R;                   // code rate (R=K/N)
};
void init_params(params &p);

struct modules
{
	std::unique_ptr<spu::module::Source_random<>>          source;
	std::unique_ptr<module::Encoder_RS<>> encoder;
	std::unique_ptr<module::Modem_BPSK<>>             modem;
	std::unique_ptr<module::Channel_AWGN_LLR<>>       channel;
	std::unique_ptr<module::Decoder_RS_std<>> decoder;
	std::unique_ptr<module::Monitor_BFER<>>           monitor;
};
void init_modules(const params &p, modules &m);

struct buffers
{
	std::vector<int  > ref_bits;
	std::vector<int  > enc_bits;
	std::vector<float> symbols;
	std::vector<float> sigma;
	std::vector<float> noisy_symbols;
	std::vector<float> LLRs;
	std::vector<int  > dec_bits;
};
void init_buffers(const params &p, buffers &b);

struct utils
{
	std::unique_ptr<tools::Sigma<>>               noise;     // a sigma noise type
	std::vector<std::unique_ptr<spu::tools::Reporter>> reporters; // list of reporters dispayed in the terminal
	std::unique_ptr<spu::tools::Terminal_std>          terminal;  // manage the output text in the terminal
};
void init_utils(const modules &m, utils &u);

int main(int argc, char** argv)
{
	// get the AFF3CT version
	const std::string v = "v" + std::to_string(tools::version_major()) + "." +
	                            std::to_string(tools::version_minor()) + "." +
	                            std::to_string(tools::version_release());

	std::cout << "#----------------------------------------------------------"      << std::endl;
	std::cout << "# This is a basic program using the AFF3CT library (" << v << ")" << std::endl;
	std::cout << "# Feel free to improve it as you want to fit your needs."         << std::endl;
	std::cout << "#----------------------------------------------------------"      << std::endl;
	std::cout << "#"                                                                << std::endl;

	params p;  init_params (p   ); // create and initialize the parameters defined by the user
	modules m; init_modules(p, m); // create and initialize the modules
	buffers b; init_buffers(p, b); // create and initialize the buffers required by the modules
	utils u;   init_utils  (m, u); // create and initialize the utils

	// display the legend in the terminal
	u.terminal->legend();

	// loop over the various SNRs
	for (auto ebn0 = p.ebn0_min; ebn0 < p.ebn0_max; ebn0 += p.ebn0_step)
	{
		// compute the current sigma for the channel noise
		const auto esn0 = tools::ebn0_to_esn0(ebn0, p.R);
		std::fill(b.sigma.begin(), b.sigma.end(), tools::esn0_to_sigma(esn0));

		u.noise->set_values(b.sigma[0], ebn0, esn0);

		// display the performance (BER and FER) in real time (in a separate thread)
		u.terminal->start_temp_report();

		// run the simulation chain
		while (!m.monitor->fe_limit_achieved() /*&& !u.terminal->is_interrupt()*/)
		{
			m.source ->generate    (                          b.ref_bits     );
            std::cout << "[1]\n";
			m.encoder->encode      (         b.ref_bits,      b.enc_bits     );
            std::cout << "[2]\n";
			m.modem  ->modulate    (         b.enc_bits,      b.symbols      );
            std::cout << "[3]\n";
			m.channel->add_noise   (b.sigma, b.symbols,       b.noisy_symbols);
            std::cout << "[4]\n";
			m.modem  ->demodulate  (b.sigma, b.noisy_symbols, b.LLRs         );
            std::cout << "[5]\n";
			m.decoder->decode_siho (         b.LLRs,          b.dec_bits     );
            std::cout << "[6]\n";
			m.monitor->check_errors(         b.dec_bits,      b.ref_bits     );
		}

		// display the performance (BER and FER) in the terminal
		u.terminal->final_report();

		// reset the monitor for the next SNR
		m.monitor->reset();
		//u.terminal->reset();

		// if user pressed Ctrl+c twice, exit the SNRs loop
		//if (u.terminal->is_over()) break;
	}

	std::cout << "# End of the simulation" << std::endl;

	return 0;
}

void init_params(params &p)
{
	p.R = (float)p.K / (float)p.N;
	std::cout << "# * Simulation parameters: "              << std::endl;
	std::cout << "#    ** Frame errors   = " << p.fe        << std::endl;
	std::cout << "#    ** Noise seed     = " << p.seed      << std::endl;
	std::cout << "#    ** Info. bits (K) = " << p.K         << std::endl;
	std::cout << "#    ** Frame size (N) = " << p.N         << std::endl;
	std::cout << "#    ** Code rate  (R) = " << p.R         << std::endl;
	std::cout << "#    ** SNR min   (dB) = " << p.ebn0_min  << std::endl;
	std::cout << "#    ** SNR max   (dB) = " << p.ebn0_max  << std::endl;
	std::cout << "#    ** SNR step  (dB) = " << p.ebn0_step << std::endl;
	std::cout << "#"                                        << std::endl;
}

void init_modules(const params &p, modules &m)
{
    // const int t = 10; // Количество исправляемых ошибок (подберите нужное)
    // const int n_rdncy = p.N - p.K; // Для BCH: n_rdncy ≈ t*m, где m - степень расширения поля

     // // Create the Galois Field generator first
    const tools::RS_polynomial_generator GF(p.N, p.t); // You'll need to define m and n_rdncy in your params

	m.source  = std::unique_ptr<spu::module::Source_random    <>>(new spu::module::Source_random         <>(p.K * 7));
    std::cout << "[1]\n";
	m.encoder = std::unique_ptr<module::Encoder_RS            <>>(new module::Encoder_RS<>(p.K, p.N, GF));
    std::cout << "[2]\n";
	m.modem   = std::unique_ptr<module::Modem_BPSK            <>>(new module::Modem_BPSK            <>(p.N));
    std::cout << "[3]\n";
	m.channel = std::unique_ptr<module::Channel_AWGN_LLR      <>>(new module::Channel_AWGN_LLR      <>(p.N));
    std::cout << "[4]\n";
	m.decoder = std::unique_ptr<module::Decoder_RS_std        <>>(new module::Decoder_RS_std<>(p.K, p.N, GF));
    std::cout << "[5]\n";
	m.monitor = std::unique_ptr<module::Monitor_BFER          <>>(new module::Monitor_BFER          <>(p.K, p.fe));
	m.channel->set_seed(p.seed);
};

void init_buffers(const params &p, buffers &b)
{
	b.ref_bits      = std::vector<int  >(p.K * 7);
	b.enc_bits      = std::vector<int  >(p.N * 7);
	b.symbols       = std::vector<float>(p.N * 7);
	b.sigma         = std::vector<float>(  1);
	b.noisy_symbols = std::vector<float>(p.N);
	b.LLRs          = std::vector<float>(p.N);
	b.dec_bits      = std::vector<int  >(p.K);
}

void init_utils(const modules &m, utils &u)
{
	// create a sigma noise type
	u.noise = std::unique_ptr<tools::Sigma<>>(new tools::Sigma<>());
	// report the noise values (Es/N0 and Eb/N0)
	u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new tools::Reporter_noise<>(*u.noise)));
	// report the bit/frame error rates
	u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new tools::Reporter_BFER<>(*m.monitor)));
	// report the simulation throughputs
	u.reporters.push_back(std::unique_ptr<spu::tools::Reporter>(new tools::Reporter_throughput<>(*m.monitor)));
	// create a terminal that will display the collected data from the reporters
	u.terminal = std::unique_ptr<spu::tools::Terminal_std>(new spu::tools::Terminal_std(u.reporters));
}