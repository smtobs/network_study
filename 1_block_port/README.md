# network_study
네트워크 관련 기술을 배우기 위한 곳입니다.

## 1_block_port
넷필터를 이용하여 특정 포트에 들어오는 패킷을 차단하는 기능을 구현.

### 넷필터란 ?
netfilter은 kernel 내에 있는 네트워크 관련 프레임워크이다. 먼 저 각각의 프로토콜은 hook point라는 것을 정의하며, 이는 패킷 프로토콜 스택의 packet's traversal에 있는 잘 정의된 포인터를 의미한다. 이러한 포인터에서, 각각의 프로토콜은 패킷과 hook number를 이용하여 넷필터 프레임워크를 호출하게 된다. 따라서 패킷이 넷필터 프레임 워크를 통과할 때, 누가 그 프로토콜과 훅을 등록했는지 확인하게 된다. 이러한 것이 등록되어 있다면, 등록된 순서대로 패킷을 검사하고, 패킷을 무시하거나 통과시키고 넷필터에게 지시하거나(NF_STOLEN), 사용 자 공간에 패킷을 대기시키도록(queuing) 넷필터에게 요청한다(NF_QUEUE). Ipv4의 netfilter에는 총 5개의 hook point가 존재 한다.

<img width="585" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/ad152763-7bfc-4c45-ad31-a87098d3cc14">

<img width="296" alt="image" src="https://github.com/smtobs/network_study/assets/50127167/caa64e81-ddf5-4a03-ac74-2a4d2aa5f4dc">
