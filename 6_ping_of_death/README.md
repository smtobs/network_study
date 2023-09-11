## 6_ping_of_death
ping of death 공격을 방어하기 위한 ICMP 모니터링 어플리케이션과 ICMP 필터 드라이버를 설계. 

### ping_of death란 ?
규정 크기 이상의 ICMP 패킷으로 시스템을 마비시키는 공격을 말한다. Ping명령을 실행하면 ICMP Echo Request 패킷을 원격 IP 주소에 송신하고 ICMP 응답을 기다린다. 대표적인 것이 버퍼 크기를 초과하는 핑 패킷으로 공격대상의 IP 스택을 넘치게 하는 것이다.

<img width="416" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/6afa1ca6-c1bb-48bb-bb43-51370d59dbc3">
