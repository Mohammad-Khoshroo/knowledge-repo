# DMA Testbench

این testbench برای تست کردن DMA با استفاده از shared bus و memory modules طراحی شده است.

## ویژگی‌ها

- **DMA Controller**: کنترل‌کننده DMA با قابلیت انتقال داده بین memory ها
- **Shared Bus**: Bus مشترک برای ارتباط بین master ها و slave ها
- **Memory Modules**: ماژول‌های حافظه با قابلیت خواندن و نوشتن
- **Testbench**: تست‌های جامع برای بررسی عملکرد DMA

## ساختار فایل‌ها

```
DMA/
├── DMA.cpp                    # پیاده‌سازی DMA
├── complex_embedded_bus.hpp   # Shared bus implementation
├── master_module.hpp          # Master module interface
├── slave_module.hpp           # Slave module interface
├── memory.hpp                 # Memory module implementation
├── utils.hpp                  # Utility classes and types
├── DMA_testbench.cpp         # Testbench implementation
├── Makefile                  # Build configuration
└── README.md                 # این فایل
```

## نحوه استفاده

### 1. تنظیم SystemC

قبل از استفاده، مطمئن شوید که SystemC نصب شده است:

```bash
# برای Ubuntu/Debian
sudo apt-get install systemc-dev

# یا از منبع
wget https://www.accellera.org/images/downloads/standards/systemc/systemc-2.3.3.tar.gz
tar -xzf systemc-2.3.3.tar.gz
cd systemc-2.3.3
mkdir build && cd build
../configure --prefix=/usr/local/systemc-2.3.3
make -j4
sudo make install
```

### 2. تنظیم متغیر محیطی

```bash
export SYSTEMC_HOME=/usr/local/systemc-2.3.3
export LD_LIBRARY_PATH=$SYSTEMC_HOME/lib-linux64:$LD_LIBRARY_PATH
```

### 3. کامپایل و اجرا

```bash
# کامپایل
make

# یا کامپایل و اجرا
make run

# پاک کردن فایل‌های build
make clean

# پاک کردن همه فایل‌های تولید شده
make distclean
```

## تست‌های انجام شده

### Test 1: Memory to Memory Transfer (WR Operation)
- انتقال داده از source memory به destination memory
- تنظیم control register برای WR operation
- بررسی وضعیت done signal

### Test 2: Memory to Memory Transfer (RD Operation)
- انتقال داده از destination memory به source memory
- تنظیم control register برای RD operation
- بررسی وضعیت done signal

### Test 3: Data Verification
- خواندن داده‌های منتقل شده
- مقایسه با داده‌های اصلی
- نمایش نتایج

## تنظیمات DMA

### Control Register (Address: 0x2000)
```
Bit 0: Start signal
Bit 1: WR operation (Memory -> MMA)
Bit 2: RD operation (MMA -> Memory)
```

### Configuration Registers
- **Address 0x2001**: Source Address (fromAddress)
- **Address 0x2002**: Destination Address (toAddress)
- **Address 0x2003**: Number of Words (wordsNum)
- **Address 0x2004**: Done Status (is_done)

## خروجی‌ها

### فایل‌های CSV
- `source_memory.csv`: داده‌های source memory
- `dest_memory.csv`: داده‌های destination memory
- `source_dump.csv`: dump های source memory
- `dest_dump.csv`: dump های destination memory

### Console Output
- وضعیت انتقال‌ها
- داده‌های خوانده شده
- پیام‌های خطا و هشدار

## عیب‌یابی

### خطاهای رایج

1. **SystemC not found**: مطمئن شوید که SYSTEMC_HOME درست تنظیم شده است
2. **Compilation errors**: بررسی کنید که همه header files موجود هستند
3. **Runtime errors**: بررسی کنید که فایل‌های CSV درست ایجاد شده‌اند

### Debugging Tips

```bash
# کامپایل با debug symbols
make CXXFLAGS="-std=c++11 -Wall -Wextra -O0 -g"

# اجرا با gdb
gdb ./dma_testbench
```

## مشارکت

برای بهبود این testbench:
1. تست‌های بیشتری اضافه کنید
2. Performance metrics اضافه کنید
3. Coverage analysis اضافه کنید
4. Documentation بهبود دهید 