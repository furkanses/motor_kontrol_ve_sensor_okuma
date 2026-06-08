import pandas as pd
import numpy as np
from scipy.signal import savgol_filter, medfilt

# Genel Ortak Parametreler
ZAMAN_BASLANGIC_MS = 15950
ZAMAN_BITIS_MS = 25805

# Dosya Yolları (Kendi bilgisayarınıza göre güncelleyebilirsiniz)
INPUT_ARABA = r"C:\Users\Furkan\Desktop\loglar\araba_ldc_20260606_182601.csv"
INPUT_TUP = r"C:\Users\Furkan\Desktop\loglar\tup_ldc_20260606_182601.csv"
INPUT_MODBUS = r"C:\Users\Furkan\Desktop\loglar\modbus_20260606_182601.csv"

def modbus_veri_isleme(input_path, start_ms, end_ms):
    """Modbus verilerindeki mesajları siler, zamanı kesip sıfırlar ve kanalları ayırır"""
    print(f"\n--- MODBUS BASINÇ VERİLERİ İŞLENİYOR ---")
    
    df = pd.read_csv(input_path)
    
    # 1. Metinsel EVENT'leri silmek için sütunları sayısal tipe zorla
    basinc_sutunlari = ['OnYanma(bar)', 'ArkaYanma(bar)', 'N2O(bar)']
    for col in basinc_sutunlari:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')
            
    # Mesaj satırlarında bu değerler boş (NaN) kalacağı için o satırları komple temizle
    df = df.dropna(subset=basinc_sutunlari).copy()
    
    # 2. İstenen zaman aralığını kes (böl)
    df_filtered = df[(df['Zaman(ms)'] >= start_ms) & (df['Zaman(ms)'] <= end_ms)].copy()
    
    # 3. Zaman eksenini sıfırla
    df_filtered['Zaman(ms)'] = df_filtered['Zaman(ms)'] - start_ms
    
    # 4. Sinyal filtrelemesi YAPMADAN her kanalı ayrı ayrı dosyalara kaydet
    base_name = input_path[:-4]
    
    # Ayrıştırılacak her veri kanalı için döngü
    for col in basinc_sutunlari:
        if col in df_filtered.columns:
            # Temiz isimlendirme için parantezleri kaldırarak yeni dosya adı üret
            temiz_isim = col.replace('(bar)', '')
            output_single_path = f"{base_name}_{temiz_isim}_filtrelenmis.csv"
            
            # Sadece Zaman(ms) ve İlgili Basınç sütununu al
            df_single = df_filtered[['Zaman(ms)', col]].copy()
            df_single.to_csv(output_single_path, index=False)
            print(f"{col} kanalı başarıyla ayrıştırıldı -> {output_single_path}")

def araba_veri_isleme(input_path, start_ms, end_ms):
    """
    Araba Loadcell verisindeki yüksek frekanslı mekanik gürültüyü temizler.
    Doğrudan Savitzky-Golay filtresi kullanır.
    """
    output_path = input_path[:-4] + "_filtrelenmis.csv"
    print(f"\n--- ARABA VERİSİ İŞLENİYOR ---")
    
    df = pd.read_csv(input_path)
    
    # 1. Metinsel logları temizle ve sayısal yap
    df['ArabaLoadcell'] = pd.to_numeric(df['ArabaLoadcell'], errors='coerce')
    df = df.dropna(subset=['ArabaLoadcell']).copy()
    
    # 2. Zaman aralığını filtrele
    df_filtered = df[(df['Zaman(ms)'] >= start_ms) & (df['Zaman(ms)'] <= end_ms)].copy()
    
    # 3. Zamanı sıfırla
    df_filtered['Zaman(ms)'] = df_filtered['Zaman(ms)'] - start_ms
    
    # 4. Filtre Uygula: Araba için doğrudan Savitzky-Golay yeterlidir
    df_filtered['ArabaLoadcell_Filtreli'] = savgol_filter(df_filtered['ArabaLoadcell'], window_length=21, polyorder=3)
    
    # 5. Kaydet
    df_filtered.to_csv(output_path, index=False)
    print(f"Araba verisi başarıyla kaydedildi -> {output_path}")


def tup_veri_isleme(input_path, start_ms, end_ms):
    """
    Tüp Loadcell verisindeki devasa anlık sıçramaları (glitch/spike) temizler.
    Önce Medyan Filtre, ardından Savitzky-Golay filtresi kombine edilir.
    """
    output_path = input_path[:-4] + "_filtrelenmis.csv"
    print(f"\n--- TÜP VERİSİ İŞLENİYOR ---")
    
    df = pd.read_csv(input_path)
    
    # 1. Metinsel logları temizle ve sayısal yap
    df['TupLoadcell'] = pd.to_numeric(df['TupLoadcell'], errors='coerce')
    df = df.dropna(subset=['TupLoadcell']).copy()
    
    # 2. Zaman aralığını filtrele
    df_filtered = df[(df['Zaman(ms)'] >= start_ms) & (df['Zaman(ms)'] <= end_ms)].copy()
    
    # 3. Zamanı sıfırla
    df_filtered['Zaman(ms)'] = df_filtered['Zaman(ms)'] - start_ms
    
    # 4. Filtre Uygula: Devasa +-8000'lik hataları yok etmek için ÖNCE Medyan Filtre (kernel_size=5)
    # Sonra kalan sinyali yumuşatmak için Savitzky-Golay filtresi.
    medyan_sinyal = medfilt(df_filtered['TupLoadcell'], kernel_size=5)
    df_filtered['TupLoadcell_Filtreli'] = savgol_filter(medyan_sinyal, window_length=15, polyorder=2)
    
    # 5. Kaydet
    df_filtered.to_csv(output_path, index=False)
    print(f"Tüp verisi başarıyla kaydedildi -> {output_path}")


if __name__ == "__main__":
    # Her iki fonksiyonu da aynı zaman parametreleriyle çalıştırıyoruz
    araba_veri_isleme(INPUT_ARABA, ZAMAN_BASLANGIC_MS, ZAMAN_BITIS_MS)
    tup_veri_isleme(INPUT_TUP, ZAMAN_BASLANGIC_MS, ZAMAN_BITIS_MS)
    modbus_veri_isleme(INPUT_MODBUS, ZAMAN_BASLANGIC_MS, ZAMAN_BITIS_MS)
    print("\nTüm işlemler başarıyla tamamlandı!")