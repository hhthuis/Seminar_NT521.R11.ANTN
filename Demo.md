# Demo (Integer Overflow → Shell)

Challenge: `pwnchall/pwnchall/main.cpp` (seminar NT521.R11.ANTN, Nhóm 08)
Target binary: `pwnchall/x64/Release/pwnchall.exe` (PE32+ x64, MSVC /O2 /WPO).

---

## 1. Demo giải (Exploit)

Code gốc (main.cpp):

```c
if (a < 0 || b < 0) {            // "Good try"
} else if (result < 0) {         // lỗi chỗ này
    system("cmd.exe");           // -> pop shell
} else { ... "RESULT" ... }
```

`result = a+b` với `int` 32-bit. Khi cộng 2 số nguyên dương mà tổng ≥ 2³¹ = 2147483648,
kết quả bị tràn (wrap-around) thành số âm: lệnh `add` 32-bit trên x86, bit 31 trở thành
bit dấu. Do đó `result < 0` đúng dù `a >= 0 && b >= 0`, đi vào nhánh `system("cmd.exe")`.

Payload:
```
a = 2147483647  (INT_MAX,  0x7FFFFFFF)
b = 1
a + b = 2147483648 = 0x80000000  ->  -2147483648  (signed)  =>  result < 0 ✔
```

Output kỳ vọng:
```
-------=[ BAD INTEGERS ]=-------
|-< Enter two numbers to add >-|

]>> 2147483647
]>> 1

[*] C: -2147483648
[*] Popped Shell
[*] Switching to interactive mode
C:\chal>whoami
...
```

Script giải: `pwnchall/exploit.py`

```bash
# local (Windows, hoặc Linux + wine)
python3 exploit.py local
# do đó có thể gửi lệnh lấy flag:
python3 exploit.py local CMD="type flag.txt"
# remote (chạy qua socat/xinetd)
python3 exploit.py remote <host> <port>
```

Môi trường được phép: binary là tài sản của nhóm đang học tập, phân tích trong thư mục repo
đã cấp phép, chạy cô lập trong container/sandbox, không nhắm bất kỳ hệ thống thật nào.

---

## 2. Đề xuất biện pháp khắc phục / phòng ngừa

**Sửa mã nguồn (thay đổi ngữ nghĩa — ưu tiên):**
- Kiểm tra tràn trước khi cộng bằng biểu thức an toàn theo chuẩn:
  ```c
  if (a <= INT_MAX - b) result = a + b;   /* an toàn khi b >= 0 */
  else result = LONG_MIN;                 /* đánh dấu overflow */
  ```
- Dùng kiểu có độ rộng lớn hơn: `long long result = (long long)a + b;` rồi kiểm tra rơi vào khoảng `[INT_MIN, INT_MAX]`.
- Dùng hàm chuyên dụng có kiểm tra: `__builtin_add_overflow(a, b, &result)` (GCC/Clang) hoặc `_addcarry_u64` / `SafeAdd` trên MSVC.
- Cung cấp hàm cộng có báo tràn theo chuẩn C (C23 có `[ ]` generated... thực tế: kiểm tra điều kiện tiền đề như trên).

**Phòng ngừa ở tầng build/CI:**
- Bật cảnh báo/analysis của trình biên dịch: MSVC `/analyze`, Clang/GCC `-Wconversion -Wall -Werror`; VS debug `/RTCc` sẽ dừng chương trình khi phát hiện tràn số nguyên lúc chạy.
- Kích hoạt UBSan (`-fsanitize=undefined -fsanitize=signed-integer-overflow`) trong build test để tự động fail khi có tràn — phát hiện sớm ở CI thay vì ngoài production.
- Chính sách lập trình: cấm phép toán số học int có thể tràn mà không qua hàm kiểm tra; xem signed overflow là UB, không dựa vào hành vi wrap của CPU.

**Giảm thiểu hậu quả nếu vẫn bị lỗi:**
- **Không gọi `system()` với chuỗi cố định không cần thiết**: `system("cmd.exe")`/`system("/bin/sh")` là "chìa khóa" nguy hiểm; nếu không bắt buộc, đừng spawn shell. Giữ quyền tối thiểu cho tiến trình (không chạy với quyền root/SYSTEM).
- Nếu phải nhận input số: parse bằng hàm trả về đầy đủ trạng thái lỗi và validate phạm vi (`strtol` + kiểm tra `errno == ERANGE` thay vì `scanf("%d")` tin cậy hoàn toàn).
- Chạy service trong sandbox (Docker, seccomp, container không có shell, flag chỉ hiển thị khi có quyền riêng) để một shell bị lấy cắp không trở thành chiếm quyền host.

---

## 3. Phân tích điều kiện, cơ chế bảo vệ & giới hạn ảnh hưởng đến khai thác

### 3.1 Điều kiện để khai thác thành công
1. **Cả 2 input hợp lệ vào `int`**: `scanf("%d")` phải parse được `a, b ∈ [-2³¹, 2³¹-1]`; không có ký tự lạ, không có giai đoạn parse lỗi (EOF/en) dừng sớm.
2. **Vượt qua cổng chặn đầu**: cần `a ≥ 0 && b ≥ 0` (nếu một số âm → nhánh "Good try", thoát).
3. **Tổng phải tràn âm**: `a + b ≥ 2³¹ = 2147483648`. Vì `a,b ≤ INT_MAX = 2³¹-1`, tổng tối đa là `2³²-2`, nên chỉ cần chọn đủ lớn. Tập hợp hợp lệ: `a,b ∈ [0, 2147483647]` và `a+b ≥ 2147483648`.
4. **Compiler phát mã wrap như máy tính**: MSVC x64 dùng lệnh `add` 32-bit → tràn tự nhiên thành bit dấu. Lưu ý đây là UB theo chuẩn C/C++: nếu build với `-fsanitize=signed-integer-overflow`, `-fwrapv` tắt, hoặc code dịch chuyển lên bigint thì hiện tượng này có thể không còn — khai thác nhạy cảm với cách build.
5. **Nhánh vulnerable phải tồn tại trong binary** (đúng là release đang cung cấp).

### 3.2 Cơ chế bảo vệ của binary và vì sao chúng KHÔNG chặn được
Kiểm tra `pwnchall.exe` (Release x64):
- **ASLR** (Dynamic base + High entropy VA) ✔
- **DEP/NX** ✔
- **/GS stack cookie** (có SecurityCookie trong Load Config) ✔
- **CFG (GuardFlags != 0)** ✔
- **/O2, /WPO, SDL check** — nhưng SDL không chuyển thành runtime guard ở đây.

Ở đây **không có memory-corruption**: không buffer overflow, không ghi trái phép, không overwrite return address.
Attacker chỉ làm giá trị `result` âm sai để đi đúng nhánh chương trình và kích hoạt lệnh gọi `system("cmd.exe")`
hợp lệ. Vì vậy:
- ASLR/DEP/CFG vô nghĩa: không có code injection, không có gadget/ROP, không có redirect control flow.
- /GS cookie chỉ bảo vệ stack frame có buffer mảng; `main` không có buffer, cookie không liên quan.
- Những lớp bảo vệ này bảo vệ "khai thác bộ nhớ", không bảo vệ "logic sai".

### 3.3 Giới hạn ảnh hưởng đến khả năng khai thác
- **Giới hạn về cách build**: nếu build với sanitizer (`-fsanitize=undefined`) hay compile với wrap-aware checks (`/RTCc`, debug), chương trình sẽ abort/không wrap → khai thác thất bại. Khai thác chỉ đúng khi sinh mã wrap 2's complement.
- **Giới hạn về môi trường chạy**: `system()` mở `cmd.exe` với quyền của tiến trình. Tiến trình chạy quyền thấp/sandbox → shell yếu; chạy **SYSTEM/root** → toàn bộ máy chủ. Do đó mức độ thành công phụ thuộc *privilege của service*, không phải kỹ thuật exploit.
- **Chiều dài payload**: chỉ 2 số nguyên, không có kênh nhập thêm sau nhánh; nếu `system` không tồn tại trong imports (build tĩnh/loại bỏ CRT runtime) thì nhánh vẫn chạy nhưng không spawn shell — khai thác phụ thuộc `system` được link (ở đây import `system` từ UCRT: có ✔).
- **Kết nối remote**: cần service wrapper (socat/xinetd) bảo toàn stdin/stdout; nếu wrapper chuyển về non-interactive pipe, `cmd.exe` có thể không nhận được command — vẫn có flag bằng lệnh gửi kèm.
- **Chống tự động hóa / re-trigger**: trạng thái chỉ phụ thuộc input, không có random, không có race — khai thác ổn định với payload cố định `INT_MAX + 1`; cơ hội thành công về lý thuyết là xác định (không mang tính xác suất).

### 3.4 Tóm tắt
- Điều kiện đủ để thắng: `a,b ≥ 0` và `a+b ≥ 2³¹` (chỉ cần `INT_MAX;1`).
- Toàn bộ cơ chế bảo vệ lớp bộ nhớ hiện diện nhưng **không ngăn được** lỗi logic;
- Cách ngăn thật sự: kiểm tra tràn ở tầng nguồn/sanitizer, không gọi `system()`, và chạy service với quyền tối thiểu trong sandbox.