# Nội dung seminar Integer overflow 

Tài liệu cho phần trình bày về khái niệm, nguyên lý, điều kiện phát sinh, mã CWE và phân tích mã nguồn. Thông điệp cần làm rõ: kiểu dữ liệu của phép tính quyết định kết quả ở biên; một giá trị quay vòng có thể trở thành chỉ số truy cập sai.

Slide 2 dùng ví dụ số học C với `unsigned int` 32 bit. Slide 5 dùng C++17 với `std::vector<int>::size_type` là kiểu unsigned 64 bit, như môi trường đã kiểm tra. Ví dụ vector rỗng minh họa **underflow/wraparound (CWE-191)**. Slide 6 dùng mini challenge C++17 tự xây dựng để minh họa **overflow vượt cận trên (CWE-190)**, với phép nhân unsigned 32 bit. Các ví dụ phục vụ phần phân tích mã nguồn trong phạm vi được giao.

## Slide và nội dung

| Slide | Nội dung | 
| --- | --- | 
| 1 | Integer overflow là gì | 
| 2 | Nguyên lý và signed so với unsigned | 
| 3 | Điều kiện trở thành lỗ hổng |
| 4 | Mã CWE và quan hệ nguyên nhân hậu quả | 
| 5 | Vector rỗng và phép trừ unsigned |
| 6 | Mini CTF với phép nhân gây integer overflow | 

## Slide 1 Integer overflow là gì

**Nội dung trên slide**

**Integer overflow (tràn số nguyên)** là hiện tượng kết quả toán học của một phép tính số nguyên vượt quá giá trị lớn nhất mà kiểu dữ liệu dùng cho phép tính có thể biểu diễn.

- **Vì sao xảy ra:** kiểu số nguyên có số bit hữu hạn, nên chỉ biểu diễn được một miền giá trị nhất định.
- **Ví dụ về giới hạn biểu diễn:** số không dấu 8 bit có miền từ 0 đến 255; kết quả toán học `255 + 1 = 256` không vừa trong 8 bit.
- **Ý nghĩa bảo mật:** kết quả tính toán sai có thể làm sai kiểm tra độ dài, kích thước cấp phát hoặc chỉ số truy cập.

Ví dụ 8 bit chỉ minh họa giới hạn biểu diễn. Cách xử lý khi vượt giới hạn phụ thuộc vào kiểu dữ liệu và ngôn ngữ, được giải thích ở slide 2.

**Nguồn chân slide:** MITRE CWE-190 [1].

## Slide 2 Nguyên lý và sự khác nhau giữa signed và unsigned

**Nội dung trên slide**

| Kiểu trong ví dụ | Miền giá trị | Khi kết quả cộng vượt giới hạn |
| --- | --- | --- |
| `uint32_t` | 0 đến 4.294.967.295 | Số học unsigned 32 bit quay vòng theo modulo 2³² |
| `int32_t` | −2.147.483.648 đến 2.147.483.647 | Tràn số học signed thông thường là undefined behavior |

```c
uint32_t u = UINT32_MAX;
u = u + 1u;                 // 0 trên mô hình đã nêu

int32_t s = INT32_MAX;
/* s = s + 1; */            // UB, không dùng làm demo kết quả
```

Minh họa nguyên lý: `4.294.967.295 + 1 = 4.294.967.296`, nhưng `4.294.967.296 mod 2³² = 0`.

**Nguồn chân slide:** SEI CERT INT30-C [2], INT32-C [3]; WG14 N1570, §6.2.5 đoạn 9 và §6.5 đoạn 5 [4]. N1570 là bản committee draft công khai của C11, không phải bản tiêu chuẩn ISO thương mại.

## Slide 3 Khi nào lỗi số học trở thành lỗ hổng

**Nội dung trên slide**

1. Một đầu vào có thể làm kết quả phép tính vượt miền biểu diễn.
2. Chương trình thiếu kiểm tra phù hợp trước khi dùng kết quả.
3. Kết quả chi phối thao tác nhạy cảm: cấp phát, sao chép, chỉ số hoặc hạn mức.
4. Đường thực thi nguy hiểm thực sự có thể đi tới từ đầu vào đó.

Ví dụ cần rà soát: `count * item_size`, `length + header_size`, `v.size() - 1` khi vector có thể rỗng.


**Nguồn chân slide:** MITRE CWE-190 [1]; ngoại lệ wrap có chủ đích trong SEI CERT INT30-C [2].

## Slide 4 Mã CWE tương ứng

**Nội dung trên slide**

| Mã | Tên chính thức | Ý nghĩa và vai trò |
| --- | --- | --- |
| **CWE-190** | Integer Overflow or Wraparound | **Mã trọng tâm:** lỗi tính toán số nguyên gây overflow hoặc wraparound ngoài dự kiến |
| CWE-131 | Incorrect Calculation of Buffer Size | Điểm yếu liên quan: tính sai kích thước bộ đệm, có thể dẫn đến cấp phát thiếu |
| CWE-787 | Out-of-bounds Write | Hậu quả có thể xảy ra: ghi dữ liệu ngoài phạm vi vùng nhớ hợp lệ |

**Một chuỗi nguyên nhân và hậu quả thường gặp:**

**Tràn số khi tính kích thước (CWE-190) → kích thước bộ đệm bị tính thiếu (CWE-131) → ghi vượt vùng nhớ (CWE-787).**

CWE-190 là mã chính của chủ đề. CWE-131 và CWE-787 mô tả các điểm yếu liên quan trong chuỗi minh họa; không phải mọi integer overflow đều dẫn đến toàn bộ chuỗi này. Lỗi số học cũng có thể làm sai kiểm tra giới hạn hoặc luồng xử lý. [1][8][10]

**Nguồn chân slide:** MITRE CWE-190 [1], CWE-131 [10], CWE-787 [8].

## Slide 5 Nhận diện và phân tích lỗi trong mã nguồn

**Nội dung trên slide**

Ví dụ dùng vector rỗng. Kết quả bên dưới ứng với môi trường có `vector<int>::size_type` unsigned 64 bit và phép trừ thực hiện ở kiểu này.

```cpp
01 #include <iostream>
02 #include <vector>
03
04 int main() {
05     std::vector<int> v;
06     std::cout << (v.size() - 1) << '\n';
07 }
```

Số dòng chỉ dùng để thuyết trình, không thuộc cú pháp C++.

| Điểm cần chỉ | Phân tích |
| --- | --- |
| Dòng 05 | Tạo vector rỗng, nên `v.size()` bằng 0 |
| `v.size()` ở dòng 06 | Trả về `vector<int>::size_type`, là kiểu unsigned; `int` trong `vector<int>` chỉ là kiểu của phần tử |
| Phép trừ ở dòng 06 | Trên mô hình này, số nguyên `1` được chuyển sang unsigned 64 bit; phép trừ `0 - 1` quay vòng |
| `cout` ở dòng 06 | In kết quả unsigned rất lớn; không truy cập phần tử của vector |

**Giá trị chứng minh**

```text
v.size()                  = 0
0 - 1 theo toán học       = -1
(-1) mod 2^64             = 2^64 - 1
Ket qua duoc in           = 18446744073709551615
```

Đã kiểm tra bằng GCC/MinGW ở chế độ C++17 trên môi trường hiện tại. Nếu phép trừ dùng unsigned 32 bit, kết quả tương ứng là `4294967295`. Không khẳng định mọi môi trường đều in cùng một số; phải xét kiểu sau integer promotions và các phép chuyển kiểu số học. [9]

**Liên hệ với nguyên nhân lỗ hổng**

Riêng chương trình trên chỉ minh họa **unsigned underflow/wraparound**, là hành vi số học được định nghĩa. Khi lập trình viên coi `v.size() - 1` là chỉ số phần tử cuối mà không xét vector rỗng, kết quả trở thành một chỉ số không hợp lệ. Ví dụ, nếu thêm dòng sau thì có truy cập ghi ngoài phạm vi; chỉ phân tích, không thực thi:

```cpp
// v[v.size() - 1] = 42;  // vector rong: khong co phan tu hop le
```

Trong C++17, truy cập này là undefined behavior; không bảo đảm sẽ crash hoặc khai thác được. Vector rỗng vốn không có phần tử hợp lệ. Cần có thêm đường đi từ đầu vào và thao tác sử dụng sai mới kết luận được về lỗ hổng thực tế. [7][8]

**Chốt nguyên nhân gốc:** khi mục tiêu là tìm chỉ số cuối, giả định vector không rỗng là điều kiện còn thiếu. Phép toán cần chú ý là `v.size() - 1` ở dòng 06. `cout` chỉ cho thấy kết quả; thao tác dùng chỉ số không hợp lệ mới tạo ra lỗi truy cập bộ nhớ.


**Nguồn chân slide:** MITRE CWE-191 [6]; Microsoft Learn, `vector::size`, `size_type`, `operator[]` [7]; WG21 N4861, `[basic.fundamental]` và `[expr.arith.conv]` [9].

## Slide 6 Mini CTF với phép nhân gây integer overflow

**Nội dung trên slide**

**Câu hỏi:** “Nhập số nguyên không dấu `n` khác 0 nào để chương trình in `WIN!`?”

Mini challenge tự xây dựng để phân tích mã nguồn. Giả định `unsigned int` và `std::uint32_t` đều là kiểu unsigned 32 bit, như môi trường GCC/MinGW đang dùng; phép nhân bên dưới được thực hiện ở 32 bit.

```cpp
01 #include <cstdint>
02 #include <iostream>
03
04 int main() {
05     std::uint32_t n = 0;
06     if (!(std::cin >> n)) return 0;
07     std::uint32_t total = n * 4u;
08     if (n != 0 && total == 0)
09         std::cout << "WIN!\n";
10     else
11         std::cout << "TRY AGAIN\n";
12 }
```

Số dòng chỉ dùng để thuyết trình. Dòng 06 kiểm tra việc đọc số thành công; nó không kiểm tra tích ở dòng 07 có vừa 32 bit hay không.

**Một đáp án và phép tính chứng minh**

```text
n                    = 1073741824 = 2^30
n * 4 theo toán học  = 4294967296 = 2^32
total trong uint32_t = 2^32 mod 2^32 = 0

n != 0       -> true
total == 0   -> true
Ket qua      -> WIN!
```

Giá trị `1073741824` tự nó vẫn vừa `uint32_t`; chính **kết quả phép nhân** mới vượt giới hạn. Đây là một đáp án, không phải đáp án duy nhất.

| Vị trí | Ý nghĩa trong phân tích |
| --- | --- |
| Dòng 06 | Người nhập điều khiển giá trị `n` |
| Dòng 07 | Phép nhân quay vòng do kết quả toán học vượt `UINT32_MAX` |
| Dòng 08 | Nhánh kiểm tra sử dụng kết quả đã quay vòng |
| Dòng 09 | Nhánh `WIN!` được thực hiện với một đầu vào khác 0 |

**Nguyên nhân gốc:** suy luận “số khác 0 nhân 4 không thể bằng 0” chỉ đúng trong phép tính số nguyên toán học, không đúng với mọi phép nhân unsigned 32 bit. Chương trình không kiểm tra miền giá trị trước khi dùng tích để quyết định nhánh thực thi.

**Mã tương ứng:** CWE-190. Ví dụ minh họa ảnh hưởng của integer overflow đến logic kiểm tra; không có cấp phát bộ đệm hoặc ghi ngoài vùng nhớ, nên không gán thêm CWE-131 hay CWE-787 cho đoạn mã này. Đây là mini challenge minh họa lý thuyết, không thay thế challenge Pwnable trong phần demo chung của nhóm.


**Nguồn chân slide:** MITRE CWE-190 [1]; SEI CERT INT30-C, phần Multiplication [2]; WG21 N4861, `[basic.fundamental]` [9]. Mã challenge và cách chọn đầu vào là ví dụ tự xây dựng.

## Nguồn chính thống

Các nguồn dưới đây được đối chiếu ngày **09/10/2026**. Nên đọc trước [1], [6] và các mục `size`, `size_type` của [7]. Cuối mỗi slide ghi nguồn ngắn theo hướng dẫn đã cho; danh mục cuối bài dùng tên tổ chức, tên tài liệu và liên kết. Ghi “Ví dụ minh họa” dưới mã nguồn slide 5 và “Mini challenge tự xây dựng” dưới mã nguồn slide 6.

**[1] MITRE — CWE-190 Integer Overflow or Wraparound.** Đọc Description, Common Consequences, Demonstrative Examples và Terminology Notes để định nghĩa đúng chủ đề và phân biệt thuật ngữ. Dùng cho slide 1, 3, 4 và 6.

https://cwe.mitre.org/data/definitions/190.html

**[2] Carnegie Mellon University SEI CERT — INT30-C Ensure that unsigned integer operations do not wrap.** Đọc phần mở đầu, Multiplication và Exceptions. Dùng cho unsigned wrap và điều kiện kiểm tra số học.

https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/rules/integers-int/int30-c/

**[3] Carnegie Mellon University SEI CERT — INT32-C Ensure that operations on signed integers do not result in overflow.** Đọc phần mở đầu về undefined behavior và Implementation Details. Dùng cho slide 2.

https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/rules/integers-int/int32-c/

**[4] ISO/IEC JTC1/SC22/WG14 — N1570 Committee Draft, 12 April 2011.** Bản dự thảo công khai cho C11. Tra §6.2.5 đoạn 9 về unsigned; §6.5 đoạn 5 về kết quả không biểu diễn được; §6.3.1.1 về integer promotions; §6.3.1.3 về chuyển kiểu số nguyên. Tìm bằng số điều khoản thay vì nhầm số trang PDF với số trang in.

https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf

**[5] Carnegie Mellon University SEI CERT — INT02-C Understand integer conversion rules.** Đọc Integer Promotions và Usual Arithmetic Conversions để giải thích kiểu của biểu thức trung gian và thời điểm ép kiểu.

https://cmu-sei.github.io/secure-coding-standards/sei-cert-c-coding-standard/recommendations/integers-int/int02-c/

**[6] MITRE — CWE-191 Integer Underflow (Wrap or Wraparound).** Đọc Description để phân biệt kết quả toán học thấp hơn giới hạn dưới với trường hợp overflow vượt cận trên. Dùng riêng để giải thích ví dụ ở slide 5.

https://cwe.mitre.org/data/definitions/191.html

**[7] Microsoft Learn — vector class.** Đọc các mục `size`, `size_type` và `operator[]`: kiểu trả về của số phần tử và điều kiện truy cập theo chỉ số. Dùng cho slide 5.

https://learn.microsoft.com/en-us/cpp/standard-library/vector-class?view=msvc-170

**[8] MITRE — CWE-787 Out-of-bounds Write.** Dùng cho tình huống mở rộng có thao tác ghi vào vector bằng chỉ số không hợp lệ; không gán mã này cho câu lệnh chỉ in kết quả.

https://cwe.mitre.org/data/definitions/787.html

**[9] ISO/IEC JTC1/SC22/WG21 — N4861 Working Draft, Standard for Programming Language C++.** Bản dự thảo công khai năm 2020. Tra `[basic.fundamental]` về unsigned modulo; `[expr.arith.conv]` về quy tắc chuyển kiểu trước phép tính; `[container.requirements.general]` về `size_type` và `size()`. Các quy tắc được dùng ở đây cũng áp dụng cho ví dụ C++17. Đây là bản dự thảo, không phải bản tiêu chuẩn ISO thương mại.

https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/n4861.pdf

**[10] MITRE — CWE-131 Incorrect Calculation of Buffer Size.** Đọc Description và Demonstrative Examples để hiểu điểm yếu tính sai kích thước bộ đệm và mối liên hệ với lỗi số học. Dùng cho slide 4.

https://cwe.mitre.org/data/definitions/131.html

