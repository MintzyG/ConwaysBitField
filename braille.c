#include <wchar.h>
#include <locale.h>

int main() {
  setlocale(LC_ALL, "");
  for(int i = 10240; i <= 10495; i++){
    wchar_t caractere = (wchar_t)i;
    wprintf(L"%d - %lc\n", i, caractere);
  }
}
