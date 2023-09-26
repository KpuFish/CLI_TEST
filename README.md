# CLI_TEST

테스트 환경

- stm32cubeide (gcc compiler)  
- vs code / source tree / tera term
- stm32f401re - nuecleo


*stm32f401re nucleo 보드 Command line interface test 입니다*

*미리 등록된 명령어를 테라텀에서 치면 원하는 내용 또는 기능을 출력하거나 수행합니다*

*x-modem으로 fw 업데이트가 가능합니다*

*bootloader project와 main f/w project를 분리해놓았습니다*

*stm32f401 flash memory 구조 상태*
- 0x0800 0000 -> bootloader 시작
- 0x0801 0000 -> f/w tag 영역으로 고정 할당 (linker script section 추가)
- 0x0802 0000 -> main app f/w 진입


![image](https://github.com/KpuFish/CLI_TEST/assets/43401975/75db525a-5315-40e7-8bc4-7220d440d92c)
