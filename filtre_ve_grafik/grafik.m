%% MATLAB Motor Test Verileri Grafikleştirme Scripti
clc; clear; close all;

% --- DOSYA İSİMLERİ ---
% MATLAB'ı bu CSV dosyalarının bulunduğu klasörde çalıştırın veya tam yollarını yazın
arabaFile    = 'araba_ldc_20260606_182601_filtrelenmis.csv';
tupFile      = 'tup_ldc_20260606_182601_filtrelenmis.csv';
onYanmaFile  = 'modbus_20260606_182601_OnYanma_filtrelenmis.csv';
arkaYanmaFile= 'modbus_20260606_182601_ArkaYanma_filtrelenmis.csv';
n2oFile      = 'modbus_20260606_182601_N2O_filtrelenmis.csv';

% --- VERİLERİN OKUNMASI ---
% 'preserve' kuralı sütun adlarındaki parantezlerin ve özel karakterlerin korunmasını sağlar
opts = 'VariableNamingRule'; val = 'preserve';
dataAraba     = readtable(arabaFile, opts, val);
dataTup       = readtable(tupFile, opts, val);
dataOnYanma   = readtable(onYanmaFile, opts, val);
dataArkaYanma = readtable(arkaYanmaFile, opts, val);
dataN2O       = readtable(n2oFile, opts, val);

%% FİGÜR 1: LOADCELL (YÜK HÜCRESİ) VERİLERİ (Ham vs Filtreli)
figure('Name', 'Loadcell Kuvvet Analizi', 'NumberTitle', 'off', 'Color', 'w');

% --- 1. Alt Grafik: Araba Loadcell ---
subplot(2, 1, 1);
plot(dataAraba.("Zaman(ms)"), dataAraba.("ArabaLoadcell"), 'Color', [0.7 0.7 0.7], 'LineWidth', 1); % Ham veri gri
hold on;
plot(dataAraba.("Zaman(ms)"), dataAraba.("ArabaLoadcell_Filtreli"), 'r-', 'LineWidth', 2); % Filtreli veri kırmızı
hold off;
title('Araba Loadcell Kuvvet Grafiği');
xlabel('Zaman (ms)');
ylabel('Kuvvet');
legend('Ham Veri (Gürültülü)', 'Filtrelenmiş Veri', 'Location', 'best');
grid on; grid minor;

% --- 2. Alt Grafik: Tüp Loadcell ---
subplot(2, 1, 2);
plot(dataTup.("Zaman(ms)"), dataTup.("TupLoadcell"), 'Color', [0.7 0.7 0.7], 'LineWidth', 1); % Ham veri gri
hold on;
plot(dataTup.("Zaman(ms)"), dataTup.("TupLoadcell_Filtreli"), 'b-', 'LineWidth', 2); % Filtreli veri mavi
hold off;
title('Tüp Loadcell Kuvvet Grafiği (Anlık Sıçramalar Temizlenmiş)');
xlabel('Zaman (ms)');
ylabel('Kuvvet');
legend('Ham Veri (Sıçramalı)', 'Filtrelenmiş Veri (Medyan+Saf MA)', 'Location', 'best');
ylim([-20, 20]);
grid on; grid minor;


%% FİGÜR 2: MODBUS BASINÇ VERİLERİ (Filtresiz Orijinal Kanallar)
figure('Name', 'Modbus Basınç Analizi', 'NumberTitle', 'off', 'Color', 'w');

% --- 1. Alt Grafik: Ön Yanma Basıncı ---
subplot(3, 1, 1);
plot(dataOnYanma.("Zaman(ms)"), dataOnYanma.("OnYanma(bar)"), 'g-', 'LineWidth', 1.5);
title('Ön Yanma Odası Basıncı');
xlabel('Zaman (ms)');
ylabel('Basınç (bar)');
grid on; grid minor;

% --- 2. Alt Grafik: Arka Yanma Basıncı ---
subplot(3, 1, 2);
plot(dataArkaYanma.("Zaman(ms)"), dataArkaYanma.("ArkaYanma(bar)"), 'm-', 'LineWidth', 1.5);
title('Arka Yanma Odası Basıncı');
xlabel('Zaman (ms)');
ylabel('Basınç (bar)');
grid on; grid minor;

% --- 3. Alt Grafik: N2O Tank Basıncı ---
subplot(3, 1, 3);
plot(dataN2O.("Zaman(ms)"), dataN2O.("N2O(bar)"), 'k-', 'LineWidth', 1.5);
title('N2O Tank Basıncı');
xlabel('Zaman (ms)');
ylabel('Basınç (bar)');
grid on; grid minor;

msgbox('Grafikler başarıyla çizdirildi!', 'Başarılı');