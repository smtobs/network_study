# ping of death 방어 S/W
- ping of death 공격을 방어하기 위해 특정 시간 동안 토탈 ICMP 사이즈가 일정 크기를 초과하면 공격 대상 IP를 전부 일정 시간동안 ICMP 패킷을 차단하는 시스템을 구현

## ping of death란 ?
<img width="372" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/a9160caa-efeb-4977-b296-d44dc915a604">

규정 크기 이상의 ICMP 패킷으로 시스템을 마비시키는 공격을 말한다. Ping명령을 실행하면 ICMP Echo Request 패킷을 원격 IP 주소에 송신하고 ICMP 응답을 기다린다. 대표적인 것이 버퍼 크기를 초과하는 핑 패킷으로 공격대상의 IP 스택을 넘치게 하는 것이다.

## 개발환경
- OS : Ubuntu 20.04.6 LTS, Kernel 5.15.0
- program language : C
  
## S/W 구조
<img width="630" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/9996cacd-21ac-4b13-a2a4-bde1f1d32fb0">

1) libpcap 라이브러리를 이용하여 모니터링 하고자 하는 인터페이스를 설정 후 패킷을 캡쳐
2) ICMP 헤더 타입이 ICMP_ECHO이면 ICMP 토탈 사이즈와 ICMP를 보낸 IP주소들을 filter_list 변수와 구조체에서 관리
3) 10초 동안 ICMP 패킷 사이즈가 SIZE_THRESHOLD(8000byte) 를 초과하면 ioctl 이벤트 발생 그렇지 않을 경우에는 ICMP 토탈 사이즈 변수, filter list 구조체에 등록 된 IP를 초기화
4) icmp_filter 드라이버에서는 "IOCTL_CMD_ICMP_FILTER_REQ" ioctl 요청이 오면 netfilter hook이 등록되어 있지 않으면 netfilter hook, 차단 리스트 등록.

## 참조

### ICMP 헤더
<img width="554" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/56290079-fdcb-4685-8fb7-8ec1c56d25f8">
