%% Signal Processing Utilities
% Collezione di funzioni per elaborazione segnali e analisi spettrale
% 
% Dipendenze: Signal Processing Toolbox
% Complessità: varia per funzione (specificata in ogni caso)

function [filtered_signal, h] = butterworth_filter(signal, fs, fc, order, filter_type)
    % Applica filtro Butterworth al segnale
    %
    % Input:
    %   signal      - Segnale di input (vettore)
    %   fs          - Frequenza di campionamento (Hz)
    %   fc          - Frequenza di taglio (Hz) o [fc_low, fc_high] per bandpass/bandstop
    %   order       - Ordine del filtro
    %   filter_type - 'low', 'high', 'bandpass', 'stop'
    %
    % Output:
    %   filtered_signal - Segnale filtrato
    %   h              - Risposta in frequenza del filtro
    %
    % Complessità: O(N log N) dove N è la lunghezza del segnale
    
    % Validazione input
    if nargin < 5
        filter_type = 'low';
    end
    
    % Normalizza frequenze (Nyquist = 1)
    nyquist = fs / 2;
    
    if strcmp(filter_type, 'bandpass') || strcmp(filter_type, 'stop')
        if length(fc) ~= 2
            error('Per filtri bandpass/stop servono due frequenze [fc_low, fc_high]');
        end
        wn = fc / nyquist;
    else
        wn = fc / nyquist;
    end
    
    % Progetta filtro
    [b, a] = butter(order, wn, filter_type);
    
    % Applica filtro (zero-phase filtering)
    filtered_signal = filtfilt(b, a, signal);
    
    % Calcola risposta in frequenza se richiesta
    if nargout > 1
        [h, w] = freqz(b, a, 1024, fs);
    end
    
    % Plot di controllo
    if nargout == 0
        figure;
        subplot(2,1,1);
        plot([1:length(signal)]/fs, signal, 'b', [1:length(filtered_signal)]/fs, filtered_signal, 'r');
        title('Segnale originale vs filtrato');
        xlabel('Tempo (s)'); ylabel('Ampiezza');
        legend('Originale', 'Filtrato');
        
        subplot(2,1,2);
        [h, w] = freqz(b, a, 1024, fs);
        semilogx(w, 20*log10(abs(h)));
        title('Risposta in frequenza del filtro');
        xlabel('Frequenza (Hz)'); ylabel('Magnitudo (dB)');
        grid on;
    end
end

function [psd_est, f] = welch_psd(signal, fs, window_length, overlap_percent)
    % Stima PSD usando metodo di Welch
    %
    % Input:
    %   signal         - Segnale di input
    %   fs            - Frequenza di campionamento
    %   window_length - Lunghezza finestra (campioni), default: length(signal)/8
    %   overlap_percent - Sovrapposizione percentuale, default: 50
    %
    % Output:
    %   psd_est - Stima della densità spettrale di potenza
    %   f       - Vettore delle frequenze
    %
    % Complessità: O(N log N)
    
    if nargin < 3
        window_length = round(length(signal) / 8);
    end
    if nargin < 4
        overlap_percent = 50;
    end
    
    overlap_samples = round(window_length * overlap_percent / 100);
    
    % Usa finestra di Hann per ridurre leakage spettrale
    window = hann(window_length);
    
    [psd_est, f] = pwelch(signal, window, overlap_samples, [], fs);
    
    % Plot se nessun output richiesto
    if nargout == 0
        figure;
        semilogy(f, psd_est);
        title('Stima PSD - Metodo di Welch');
        xlabel('Frequenza (Hz)');
        ylabel('PSD (dB/Hz)');
        grid on;
    end
end

function snr_db = calculate_snr(signal, noise_start, noise_end)
    % Calcola SNR di un segnale
    %
    % Input:
    %   signal     - Segnale completo
    %   noise_start - Indice di inizio del segmento di solo rumore
    %   noise_end   - Indice di fine del segmento di solo rumore
    %
    % Output:
    %   snr_db - SNR in decibel
    %
    % Complessità: O(N)
    
    % Estrai segmento di rumore
    noise_segment = signal(noise_start:noise_end);
    noise_power = mean(noise_segment.^2);
    
    % Potenza totale del segnale
    signal_power = mean(signal.^2);
    
    % SNR = (Potenza segnale + rumore - Potenza rumore) / Potenza rumore
    clean_signal_power = signal_power - noise_power;
    snr_linear = clean_signal_power / noise_power;
    snr_db = 10 * log10(snr_linear);
    
    fprintf('SNR stimato: %.2f dB\n', snr_db);
end

function peaks_info = find_spectral_peaks(signal, fs, min_prominence, min_distance)
    % Trova picchi significativi nello spettro
    %
    % Input:
    %   signal         - Segnale di input
    %   fs            - Frequenza di campionamento
    %   min_prominence - Prominenza minima dei picchi (default: auto)
    %   min_distance   - Distanza minima tra picchi in Hz (default: fs/100)
    %
    % Output:
    %   peaks_info - Struct con informazioni sui picchi
    %
    % Complessità: O(N log N)
    
    if nargin < 3
        min_prominence = [];
    end
    if nargin < 4
        min_distance = fs / 100;  % Default: 1% della fs
    end
    
    % Calcola FFT
    N = length(signal);
    Y = fft(signal);
    P = abs(Y/N).^2;
    f = (0:N-1) * fs / N;
    
    % Considera solo metà positiva dello spettro
    P = P(1:floor(N/2));
    f = f(1:floor(N/2));
    
    % Converti distanza minima in campioni
    min_distance_samples = round(min_distance * length(f) / (fs/2));
    
    % Trova picchi
    if isempty(min_prominence)
        [peak_vals, peak_locs] = findpeaks(P, 'MinPeakDistance', min_distance_samples);
    else
        [peak_vals, peak_locs] = findpeaks(P, 'MinPeakProminence', min_prominence, ...
                                          'MinPeakDistance', min_distance_samples);
    end
    
    % Prepara output
    peaks_info.frequencies = f(peak_locs);
    peaks_info.magnitudes = peak_vals;
    peaks_info.power_db = 10*log10(peak_vals);
    
    % Plot se nessun output richiesto
    if nargout == 0
        figure;
        plot(f, 10*log10(P));
        hold on;
        plot(peaks_info.frequencies, peaks_info.power_db, 'ro', 'MarkerSize', 8);
        title('Spettro con picchi identificati');
        xlabel('Frequenza (Hz)');
        ylabel('Potenza (dB)');
        legend('Spettro', 'Picchi');
        grid on;
        
        % Stampa risultati
        fprintf('Picchi trovati:\n');
        for i = 1:length(peaks_info.frequencies)
            fprintf('  %.2f Hz: %.2f dB\n', peaks_info.frequencies(i), peaks_info.power_db(i));
        end
    end
end

%% Esempio di utilizzo
% Genera segnale di test
fs = 1000;  % Hz
t = 0:1/fs:2;  % 2 secondi
f1 = 50; f2 = 120;  % Frequenze segnale
signal_clean = sin(2*pi*f1*t) + 0.5*sin(2*pi*f2*t);
noise = 0.1*randn(size(t));
signal_noisy = signal_clean + noise;

% Test delle funzioni
fprintf('=== Test Signal Processing Utils ===\n');

% Test filtro passa-basso
[signal_filtered, ~] = butterworth_filter(signal_noisy, fs, 80, 4, 'low');
fprintf('Filtro applicato.\n');

% Test PSD
[psd, f] = welch_psd(signal_noisy, fs);
fprintf('PSD calcolata.\n');

% Test SNR
noise_start = round(0.1*fs);  % Primi 0.1s come rumore
noise_end = round(0.2*fs);
snr = calculate_snr(signal_noisy, noise_start, noise_end);

% Test ricerca picchi
peaks = find_spectral_peaks(signal_noisy, fs);

fprintf('Test completato.\n');