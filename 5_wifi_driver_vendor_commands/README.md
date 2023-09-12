# RTL8812AU Wi-Fi Driver Run time 디버그 레벨 조정 기능 
- 기존 Wi-Fi 드라이버에서 디버그 레벨을 조정하려면 컴파일을 다시 해야했으나 Run Time에 디버깅 레벨을 조정할 수 있으면 개발 단계에서 유용할 것으로 보여 패치를 적용. 

## 개발환경
- Wi-Fi 어댑터 : RTL8812AU
![image](https://github.com/smtobs/network_study/assets/50127167/8362fd5a-ad92-4da2-89a0-61f9efc419b4)
- OS : Ubuntu 20.04.6 LTS, Kernel 5.15.0
- program language : C
- 패치적용 소스코드 : https://github.com/svpcom/rtl8812au.git


  
## 동작 과정
![image](https://github.com/smtobs/network_study/assets/50127167/49e4c0bc-025d-4ce4-8b38-2d403dff3969)
- iw 명령어를 사용하여 vendor cmd 요청, netlink를 통하여 메세지 전달 후 Wi-Fi Driver는 cfg80211에 등록된 wiphy_vendor_command로부터 callback 함수(.doit)을 실행하여 Runtime에서 Debug level을 조정.
![image](https://github.com/smtobs/network_study/assets/50127167/4afcf4f2-477e-44fe-9a62-858c75997c71)

### RTL8812AU Wi-Fi Driver 디버그 레벨 표
![image](https://github.com/smtobs/network_study/assets/50127167/aaa10267-3b48-4ccf-bf80-a101313e302a)


### User Guides

1. Clone this repo

   ```shell
   git clone https://github.com/smtobs/network_study.git
  
   ```

1. RTL8812AU Wi-Fi Driver source code download
   ```shell
   cd network_study/5_wifi_driver_vendor_commands
   ./build_rtl8812au.sh -init
   ```

1. build
   ```shell
   ./build_rtl8812au.sh -build
   ```

1. install
   ```shell
   ./build_rtl8812au.sh -install
   ```

1. Debug level adjust
   ```shell
   ./set_debug_level.sh [interface] [debug level]
   ```
   
