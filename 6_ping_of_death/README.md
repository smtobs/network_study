# ping of death 방어 S/W
<img width="700" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/968f7afa-fdd3-4e74-a35c-3c0ead091a10">

## ping of death란 ?
<img width="372" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/a9160caa-efeb-4977-b296-d44dc915a604">

규정 크기 이상의 ICMP 패킷으로 시스템을 마비시키는 공격을 말한다. Ping명령을 실행하면 ICMP Echo Request 패킷을 원격 IP 주소에 송신하고 ICMP 응답을 기다린다. 대표적인 것이 버퍼 크기를 초과하는 핑 패킷으로 공격대상의 IP 스택을 넘치게 하는 것이다.

## 개발환경
- Ubuntu 20.04.6 LTS
- Kernel 5.15.0
- C 언어
- libpcap, netfilter

## 테스트 환경
<img width="804" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/8ed9a581-3035-41dd-8437-fa81c10779b8">

- Laptop1 : ping of death 공격자
- Laptop2 : ping of death 공격 목표 대상

### 공격 방어 S/W 설치
- Laptop2에서 ping of death 공격을 방어하기 위하여 S/W 설치


git clone
```shell
git clone https://github.com/smtobs/network_study
```

ICMP 필터 드라이버 컴파일 및 install
```shell
cd network_study/6_ping_of_death/driver
make
sudo insmod icmp_filter.ko
```

ICMP 감시 모니터 어플리케이션 컴파일 및 install
```shell
cd network_study/6_ping_of_death/user
make
sudo ./monitor_icmp [ your interface ] 
```

### 테스트 확인
<img width="1201" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/dd4206be-3a69-471b-9acb-d507763d7c5b"> 
<img width="1200" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/5c44c799-162e-4ab8-ae7b-30ae1d6dd700">

- Wireshark로 확인한 ICMP 패킷과 ICMP 필터 드라이버 로그

- 일정 시간 안에 ICMP request 패킷이 THRESHOLD를 초과하면 icmp 필터에서 패킷을 DROP 시키는 것을 확인 할 수 있다.
  
## S/W 구조
<img width="630" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/9996cacd-21ac-4b13-a2a4-bde1f1d32fb0">

1) libpcap 라이브러리를 이용하여 모니터링 하고자 하는 인터페이스를 설정 후 패킷을 캡쳐한다
2) ICMP 헤더 타입이 ICMP_ECHO이면 ICMP 토탈 사이즈와 ICMP를 보낸 IP주소들을 filter_list 변수와 구조체에서 관리한다
3) 10초 동안 ICMP 패킷 사이즈가 SIZE_THRESHOLD(8000byte) 를 초과하면 ioctl 이벤트 발생 그렇지 않을 경우에는 ICMP 토탈 사이즈 변수, filter list 구조체에 등록 된 IP를 초기화 한다
4) icmp_filter 커널 모듈에서는 "IOCTL_CMD_ICMP_FILTER_REQ" ioctl 요청일때 netfilter hook이 등록되어 있지 않으면 netfilter hook과 차단 리스트를 등록한다(netfilter hook은 icmp_filter 함수를 등록하고 prerouting으로 등록한다)

### hook (icmp_filter 함수)
<img width="600" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/09a5d28d-8d04-4787-a33c-ab952932b294">
