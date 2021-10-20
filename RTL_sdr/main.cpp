#include "dy4.h"
#include "filter.h"
#include "genfunc.h"
#include "PLL.h"

#include <fstream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <string>
#include <iostream>
#include <string>

using namespace std;

//some global varibles
vector<float> rf_filter,audio_filter, BPF_19, BPF_22;

short int ToShort(const float a){
    if (abs(a) < 1.0) return a*32767;
    else return a > 0.0? 32767:-32768;
}


class RF_Front_End {
public:
    RF_Front_End(const RF_Front_End&) = delete;
    RF_Front_End& operator=(const RF_Front_End&) = delete;

    RF_Front_End(int size, int downsample) : m_block_size(size / 2), m_downsample_rate(downsample)
    {
        m_filter_s = N_TAPS0;
        RF_buffer_p = N_TAPS0 - 1;
        previous_i = 0;
        previous_q = 0;
        flag = true;
        i_data.resize(m_block_size + N_TAPS0 - 1);
        q_data.resize(m_block_size + N_TAPS0 - 1);
        i_filt.resize(m_block_size / downsample);
        q_filt.resize(m_block_size / downsample);
    }

    int front_end(vector<char>& data_in, vector<float>& RF_data_out, int AD_buffer_p)
    {
        for (int i = 0; i < m_block_size; i++) {
            i_data[RF_buffer_p + i] = (static_cast<uint8_t>(data_in[2 * i]) - static_cast <uint8_t>(128)) / 128.0;
            q_data[RF_buffer_p + i] = (static_cast<uint8_t>(data_in[2 * i + 1]) - static_cast <uint8_t>(128)) / 128.0;
        }
        //RF front end always mode 0
        convolveFIR(this->i_data, rf_filter, i_filt, m_block_size, 10, 0, RF_buffer_p);
        convolveFIR(this->q_data, rf_filter, q_filt, m_block_size, 10, 0, RF_buffer_p);

        FM_Demod(this->i_filt, this->q_filt, RF_data_out, AD_buffer_p);

        RF_buffer_p = flag ? 0 : (m_filter_s - 1);
        flag = !flag;
        return AD_buffer_p;
    }

    //downsample by factor of 10, demodulation
    void FM_Demod(const vector<float>& i_filt, const vector<float>& q_filt, vector<float>& RF_data_out, int& AD_buffer_p)
    {
        float derI, derQ, mag;
        int i;
        derI = i_filt[0] - previous_i;
        derQ = q_filt[0] - previous_q;
        mag = i_filt[0] * i_filt[0] + q_filt[0] * q_filt[0];
        RF_data_out[AD_buffer_p++] = (mag == 0.0) ? 0.0 : (derQ * (i_filt[0]) - derI * (q_filt[0])) / mag;

        for (i = 0; i < i_filt.size() - 1; ++i)
        {
            derI = i_filt[i + 1] - i_filt[i];
            derQ = q_filt[i + 1] - q_filt[i];
            mag = i_filt[i + 1] * i_filt[i + 1] + q_filt[i + 1] * q_filt[i + 1];
            RF_data_out[AD_buffer_p++] = (derQ * (i_filt[i]) - derI * (q_filt[i])) / mag;
        }
        previous_i = i_filt[i];
        previous_q = q_filt[i];
    }

private:
    vector<float> i_data, q_data, demod_data;
    vector<float> i_filt, q_filt;
    size_t m_filter_s;
    int m_block_size;
    int m_downsample_rate;
    int RF_buffer_p;
    bool flag;
    float previous_i;
    float previous_q;
};


class RDS {
public:
    RDS(int N_taps, float Fs, float Fs_RRC, size_t num_sample) {
        impulseResponseRootRaisedCosine(57e3, N_taps, RRC);
        impulseResponseBPF(54e3, 60e3, 240e3, N_taps, BPF54);
        impulseResponseBPF(113.5e3, 114.5e3, 240e3, N_taps, BPF114);
        impulseResponseLPF(240e3 * 19, 28e3, N_TAPS0 * 19, LPF_28);
        impulseResponseLPF(240e3, 3e3, 201, LPF_3);

        IF_sample_size = num_sample;       //IF sample size
        data_size = num_sample * 19 / 80;
        current_pos = 0;
        sampler_sync = false;
        c_prev = 0;

        RDS_mix.resize(num_sample * 2);
        RDS_carrier.resize(num_sample * 2);
        LPF_3_buffer.resize(num_sample * 2);
        LPF_28_buffer.resize(data_size * 2);        //reduce sampling rate to 57k
        Pll_in.resize(num_sample);
        NCO.resize(num_sample);
        RDS_out.resize(data_size);

        RDS_pllstate = new STATE{};
        for (float& i : LPF_28) i *= 19.0;
    }

    ~RDS() {
        delete RDS_pllstate;
    }

    int Carrier_Recovery(vector<float>& IF, int begin = 0) {
        //RDS channel extraction
        int x_next = convolveFIR(IF, BPF54, RDS_mix, IF_sample_size, 1, 0, begin, current_pos);
        //RDS Carrier Recovery       
        for (int i = current_pos; i < current_pos + IF_sample_size; i++) {
            RDS_carrier[i] = RDS_mix[i] * RDS_mix[i];
        }
        convolveFIR(RDS_carrier, BPF114, Pll_in, IF_sample_size, 1, 0, current_pos);
        fmPll(Pll_in, NCO, 114e3, 240e3, 0.5, 0.05, 0.002, RDS_pllstate);

        //mixer
        for (int i = 0; i < IF_sample_size; i++) {
            RDS_mix[current_pos + i] = NCO[i] * RDS_mix[current_pos + i];
        }

        RDS_Demod();

        current_pos = (current_pos + IF_sample_size) % RDS_mix.size();
        if (!sampler_sync) RDS_sync();
        else sampler();

        return x_next;
    }

    void RDS_Demod() {
        int k = current_pos * 19 / 80;

        convolveFIR(RDS_mix, LPF_3, LPF_3_buffer, IF_sample_size, 1, 0, current_pos, current_pos);

        //Rational resampler
        convolveFIR(LPF_3_buffer, LPF_28, LPF_28_buffer, IF_sample_size, 80, 1, current_pos, k, 19);

        //57k sampling rate
        convolveFIR(LPF_28_buffer, RRC, RDS_out, data_size, 1, 0, k);
    }
    
    void RDS_sync() {
        int i = samples_begin;
        for (; i < data_size; i += 25) {
            sync_buffer[offset++] = abs(RDS_out[i]);
            if (offset == 24) {
                offset = distance(sync_buffer.begin(), max_element(sync_buffer.begin(), sync_buffer.end()));
                sampler_sync = true;
                samples_begin = 24 + offset - (data_size - i) % 24;
                return;
            }
        }
        samples_begin += i - data_size;
    }

    void sampler() {
        char c;
        int i = samples_begin;
        for (; i < data_size; i += 24) {
            c = RDS_out[i] > 0.0 ? 1 : 0;
            RDS_symbol[RDS_symbol_p++] = c ^ c_prev;
            c_prev = c;
            if (RDS_symbol_p == RDS_symbol.size()) RDS_symbol_p = 0;
        }

        samples_begin = i - data_size;
    }
public:
    vector<unsigned char> RDS_symbol;
    int RDS_symbol_p;

private:
    bool sampler_sync;
    int samples_begin;
    int offset;
    char c_prev;
    vector<float> sync_buffer;
private:
    vector<float> RRC, BPF54, BPF114, LPF_28, LPF_3;
    vector<float> RDS_mix, RDS_out, RDS_carrier, NCO;
    vector<float> LPF_3_buffer, LPF_28_buffer, Pll_in;
    STATE* RDS_pllstate;
    int current_pos;
    size_t IF_sample_size, data_size;
};


class IO{
public:
    IO(int block_count, int audioblock_size=480, int mode=0): m_IFblock_count(block_count),m_block_size(audioblock_size), m_mode(mode)
    {
        //filter for RF front end
        impulseResponseLPF(rf_Fs0, rf_Fc, N_TAPS0, rf_filter);

        switch (mode) {
            case 0: {
                impulseResponseLPF(rf_Fs0 / rf_decim, audio_Fc, N_TAPS0, audio_filter);
                buffer_size = m_block_size * 10 * 2 * 5;
                IF_buffer_size = (m_block_size * 5) * m_IFblock_count;
                RF = new RF_Front_End(buffer_size, 10);
                m_downsample = 5;
                Create();
                break;
            }

            case 1: {
                impulseResponseLPF(rf_Fs1 / rf_decim * 24, audio_Fc, N_TAPS1, audio_filter);
                for (float& i : audio_filter) i *= 24;
                buffer_size = (m_block_size * 10 * 2 * 125) / 24;
                IF_buffer_size = (m_block_size * 125 / 24) * m_IFblock_count;
                RF = new RF_Front_End(buffer_size, 10);
                m_downsample = 125;
                Create();
                break;
            }
        }
        RDS_module = new RDS{ 151, 240e3, 57e3, IF_buffer_size / m_IFblock_count };
        m_WantTodecrement = false;
    }

    ~IO()
    {
        Destroy();
    }

    IO(const IO&) = delete;

    IO& operator=(const IO&) = delete;

    bool Create()
    {   
        try {
            buffer.resize(buffer_size);   
            RF_data_out.resize(IF_buffer_size);
        }catch(bad_alloc const&){return false;}
        
        current_write = 0;
        output_p = 0;
        m_BlockFilled = 0;
        m_ready = false;

        return true;
    }

    bool Destroy()
    {
        delete RDS_module;
        return false;
    }

    bool readstdin(char* buffer, int size) {
        cin.read(buffer, size);
        if (cin) return true;
        return false;
    }

    bool readfile(char* buffer, int size, ifstream& f) {
        if(!f.read(buffer, size))return false;
        return true;
    }
    
    virtual bool Fill_Block()
    {

        //read buffer
        m_ready = true;
        thread audio_thread(&IO::MainThread, this);
        thread RDS_thread(&IO::RDS_control, this);
        int count = 0;
        int head = 0;
        int AD_buffer_p = 0;    //the start position of IF vector
        while (m_ready) {
            while (m_BlockFilled == m_IFblock_count-1);
            if (!readstdin(&buffer[0], buffer.size())) {
                m_ready = false;
                m_BlockFilled++;
                m_cvBlockReady.notify_all();
                break;
            }
            AD_buffer_p = RF->front_end(buffer,RF_data_out, AD_buffer_p);
            AD_buffer_p = AD_buffer_p % (RF_data_out.size());
            m_BlockFilled++;
            m_cvBlockReady.notify_all();
        }
        //waiting for mainthread to finish
        while (thread_finish != 2)  m_cvBlockReady.notify_all();
        audio_thread.join();
        RDS_thread.join();
        return true;
    }

    bool RDS_control() {
        int begin = 0;
        while (m_ready) {
            if (m_BlockFilled == 0)
            {
                unique_lock<mutex> lk(m_muxBlockReady);
                m_cvBlockReady.wait(lk);
            }
            begin = RDS_module->Carrier_Recovery(RF_data_out, begin);

            m_WantTodecrement = true;
            while (m_WantTodecrement&m_ready);
        }
        thread_finish++;
        return true;
    }

    virtual void MainThread() = 0;


protected:
    int m_IFblock_count;
    size_t m_block_size;             //targe ouput audio block size
    size_t IF_buffer_size;          //size of a RF buffer(output of RF front end)
    size_t buffer_size;             //raw sample size
    vector<char> buffer;           //point to the raw buffer. (to be replaced with cin)
    vector<float> RF_data_out;
    vector<short int> data_out;
    int current_write ;             //currently unused, reserve for audio system callback
    int output_p;                   //beginning of next empty output block

protected:                          //some synchronization parameters
    atomic<bool> m_ready;
    atomic<int> m_BlockFilled;
    atomic<int> thread_finish;
    atomic<bool> m_WantTodecrement;
    mutex m_muxBlockReady;
    condition_variable m_cvBlockReady;
    uint16_t m_nBlockCurrent;
    RF_Front_End* RF;
    int m_mode;
    int m_downsample;

protected:
    RDS* RDS_module;
};


class Mono : public IO {
public:
    Mono(int block_count, int audioblock_size = 480, int mode = 0) :
        IO(block_count, audioblock_size, mode)
    {
        data_out.resize(audioblock_size * block_count);
    }

    void MainThread() override
    {
        int head = 0;
        output_p = 0;

        while (m_ready)
        {
            if (m_BlockFilled == 0)
            {
                unique_lock<mutex> lk(m_muxBlockReady);
                m_cvBlockReady.wait(lk);
            }
            //begin processing
            head = convolveFIR(RF_data_out, audio_filter, data_out, IF_buffer_size / m_IFblock_count, m_downsample, m_mode, head, output_p);

            //after proceesing one block, increase the output_p to the head of next audio block
            output_p = (output_p + m_block_size) % (data_out.size());

            if (output_p == 0) {
                fwrite(reinterpret_cast<const char*>(&data_out[0]), sizeof(short int), data_out.size(), stdout);
            }

            while (!m_WantTodecrement);
            m_WantTodecrement = false; m_BlockFilled--;
        }
        thread_finish++;
    }
};


class Stereo : public IO {
public:
    //reseve 2 block for RF front end
    //RF_out has same size as RDS_out
    Stereo(int block_count, int audioblock_size = 480, int mode = 0) :
        IO(2, audioblock_size, mode), m_stereo_block_count(2)
    {
        impulseResponseBPF(18.5e3, 19.5e3, 240e3, 151, BPF_19);
        impulseResponseBPF(22e3, 54e3, 240e3, 151, BPF_22);
        m_stereo_block_size = IF_buffer_size / m_IFblock_count;
        m_stereo_block_head = 0;
        m_stereo_block_filled = 0;
        m_process_finish = false;
        pllstate = new STATE{};

        m_stereo_out.resize(m_stereo_block_size * block_count);
        data_out.resize(audioblock_size * block_count * 2);     //L+R
        L_channel.resize(audioblock_size);
        R_channel.resize(audioblock_size);
        pllIn.resize(m_stereo_block_size);
        stereo_channel.resize(m_stereo_block_size);
        stereo_carrier.resize(m_stereo_block_size);

        Fs = mode ? 2.5e5 : 2.4e5;
    }

    ~Stereo() {
        delete pllstate;
    }

    void StereoFrontProcess() {
        int RF_head = 0;
        while (m_ready) {
            //waiting for empty block
            while (m_stereo_block_filled == m_stereo_block_count);

            if (m_BlockFilled == 0)
            {
                unique_lock<mutex> lk(m_muxBlockReady);
                m_cvBlockReady.wait(lk);
            }
            //stereo channel extraction
            convolveFIR(RF_data_out, BPF_22, stereo_channel, IF_buffer_size / m_IFblock_count, 1, 0, RF_head);

            //stereo carrier recovery
            RF_head = convolveFIR(RF_data_out, BPF_19, pllIn, IF_buffer_size / m_IFblock_count, 1, 0, RF_head);
            fmPll(pllIn, stereo_carrier, 19e3, Fs, 2.0, 0.0, 0.01, pllstate);
            
            //stereo processing
            mixer();
            m_stereo_block_head = m_stereo_block_head  % m_stereo_out.size();
            m_stereo_block_filled++;
            m_cvRDSready.notify_one();
        }
        m_process_finish = true;
        while(m_stereo_block_filled != 0)  m_cvRDSready.notify_one();
    }

    void mixer() {
        for (int i = 0; i < m_stereo_block_size; i++) {
            m_stereo_out[m_stereo_block_head++] = stereo_carrier[i] * stereo_channel[i];
        }
    }

    void combine() {
        for (int i = 0; i < m_block_size; i++) {
            data_out[output_p++] = ToShort(L_channel[i] + R_channel[i]);
            data_out[output_p++] = ToShort(L_channel[i] - R_channel[i]);
        }
    }

    void MainThread() override {
        int RDS_head = 0; //current block in process
        int RF_head = 0;
        int prev_block = 0;

        thread t2(&Stereo::StereoFrontProcess, this);

        while (!m_process_finish || m_stereo_block_filled > 0) {
            if (m_stereo_block_filled == 0) {
                unique_lock<mutex> lk(m_muxRDSready);
                m_cvRDSready.wait(lk);
            }

            //MONO path
            RF_head = convolveFIR(RF_data_out, audio_filter, L_channel, IF_buffer_size / m_IFblock_count, m_downsample, m_mode, RF_head);

            //make sure StereoFrontProcess has finished
            RDS_head = convolveFIR(m_stereo_out, audio_filter, R_channel, m_stereo_block_size, m_downsample, m_mode, RDS_head);
            prev_block = output_p;
            combine();

            //finish upgprof
            m_stereo_block_filled--;
            fwrite(reinterpret_cast<const char*>(&data_out[prev_block]), sizeof(short int), m_block_size*2, stdout);
            output_p = output_p % (data_out.size());
            
            while (!m_WantTodecrement&m_ready);
            m_WantTodecrement = false; m_BlockFilled--;
        }
        t2.join();
        thread_finish++;
    }

private:
    vector<float> m_stereo_out;
    vector<float> pllIn;
    vector<float> stereo_channel;
    vector<float> stereo_carrier;
    vector<float> L_channel;
    vector<float> R_channel;
    int m_stereo_block_count;
    int m_stereo_block_head;
    int m_stereo_block_size;
    float Fs;
    STATE* pllstate;

private:
    mutex m_muxRDSready;
    condition_variable m_cvRDSready;
    atomic<int> m_stereo_block_filled;
    bool m_process_finish;
};


int main(int argc, char* argv[])
{   
    
    int mode = 0, type = 0;
    if(argc < 2){
        cerr<<"Operating in default mode 0" << std::endl;
    }else if(argc == 2){
        mode = atoi(argv[1]);
        if (mode != 1 && mode != 0){
            cerr<<"wrong mode " << mode << endl;
            exit(1);
            }
    } else{
        type = atoi(argv[1]);
        mode = atoi(argv[2]);
        
        if (mode != 1 && mode != 0){
            cerr<<"wrong mode " << mode << endl;
            exit(1);
            }
        if (type == 0){
            cerr<<"Mono " << mode << endl;
        }
        else if(type == 1){
            cerr<<"Stereo " << mode << endl;
        }
    }
    
    if(type == 0){
        Mono audio{4,480,mode};
        audio.Fill_Block();
    }else{
        Stereo audio{4,480,mode};
        audio.Fill_Block();
    }
    
    return 0;
}
