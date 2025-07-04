# CURVIVAL
## 목차
1. [개발자](#개발자)
2. [게임 실행 영상](#게임-실행-영상)
3. [게임 실행 환경](#게임-실행-방법)
4. [게임 설정 및 스토리](#게임-설정-및-스토리)  

---

## 개발자
이 게임은 숭실대학교 1학년 3명이 김익수 교수님의 '프로그래밍1및실습'을 듣고 C언어로 만든 게임이다.
<table>
      <tr align="center">
       <td>CodeSorting<br/>
      </td>
       <td>siooooooni<br/>
      </td>
       <td>Chickeneat999<br/>
      </td>
    </tr>
    <tr align="center">
      <td style="min-width: 150px;">
            <a href="https://github.com/CodeSorting">
              <img src="https://avatars.githubusercontent.com/{CodeSorting}" width="200" alt="깃허브계정-프로필사진">
              <br />
            </a>
        </td>
      <td style="min-width: 150px;">
            <a href="https://github.com/siooooooni">
              <img src="https://avatars.githubusercontent.com/{siooooooni}" width="200" alt="깃허브계정-프로필사진">
              <br />
            </a>
        </td>
      <td style="min-width: 150px;">
            <a href="https://github.com/Chickeneat999">
              <img src="https://avatars.githubusercontent.com/{Chickeneat999}" width="200" alt="깃허브계정-프로필사진">
              <br />
            </a>
        </td>
    </tr>
    <tr align="center">
       <td>
            SSU CSE <br/>
      </td>
       <td>
            SSU CSE <br/>
      </td>
       <td>
            SSU CSE <br/>
      </td>
    </tr>
</table>

---
## 게임 실행 영상



## 게임 실행 방법
- 실행 환경 : wsl, 우분투 환경
- 설치 방법
  ```
  sudo apt update
  sudo apt upgrade
  sudo apt-get install libncurses5-dev libncursesw5-dev
  gcc Curvival.c -lncurses -lm
  ./a.out
  ```
  이미 Curvival이라는 실행 파일이 있긴 하다.
   
---

## 게임 설정 및 스토리 
C-Survival, 즉, Curvival의 설정 및 스토리는 다음과 같다.<br>

---

## 프롤로그 – 나무 인형이 도착하던 날
그날, 나는 단지 메일 하나를 열었을 뿐이었다.  
제목은 별것 아니었다.  
> “무료 성능 향상 패치! 지금 다운로드하세요.”  

첨부파일은 `TrojanHorse.exe`  
귀엽게 생긴 나무 인형 모양의 프로그램 아이콘이었다.  
의심은 들었지만, 클릭했다. 그 순간부터…  
내 컴퓨터의 ‘세상’은 뒤틀리기 시작했다.  
화면이 깜빡이고, 익숙하던 프로그램들이 오작동하더니,  
내 바탕화면 안에서… 비명이 들리기 시작했다.

---

## 설정 – 컴퓨터 속 또 다른 세계
우리가 보는 바탕화면, 앱 아이콘, 폴더…  
그 모든 것의 안쪽에는 **‘디지털 시티’**, 즉 프로그램들로 이루어진 자율 생명체들의 세상이 존재한다.  
- 그들은 메모리 블록 속에서 살아가며, CPU 스케줄을 따라 하루를 산다.
  - 마을, 산, 강, 사막, 미로바위는 모두 디지털 시티의 메모리이자 파티션이고 서로를 잇는 다리들이 존재한다.
  - 이 공간에는 수많은 프로세스 주민들이 IPC(Inter-Process Communication)를 하며 지냈다.
- 그러나 어느날 `TrojanHorse.exe`이라는 건물 하나가 들어오게 된다.  
  - 그 안에 숨어 있던 **좀비 코드**들이 깨어나 프로그램 세계를 공격하기 시작했다.  
  - 정상적인 애플리케이션, 시스템 프로세스를 감염시켜 버그화시키고,  
  - 살아남은 코드들마저 추적해 파괴한다.

---

## 적 – 좀비 코드
이들은 단순한 괴물이 아니다.  
- 한때 **메일 클라이언트**였고, **사진 뷰어**였으며, 심지어 **내 저장된 게임**이었다.  
- 그러나 지금은 감염되어 내게 달려드는 **오류 그 자체**다.  
  > “ACCESS VIOLATION AT MEMORY BLOCK `0x0000FFF`”  
- 그들에게 공격당하면, 나조차 감염될 수 있다.

---

## 사용자 – 디버깅을 결심하다
나는 안다.  
지금 이 컴퓨터가 완전히 감염되면,  
내 업무, 내 기억, 나의 삶이 전부 날아간다.  
그래서 나는 직접 컴퓨터 내부로 접속하기로 했다.  
디버깅 툴을 응급 패치해 나를 의식으로 전송한다.  
이제부터 나는 백신 프로그램 **“DEBUG”**로서 이 디지털 세상에 들어가,  
시스템을 수리하고, 감염된 좀비 코드를 정화해야 한다.

---

## 게임 목표 – D.E.B.U.G 모듈 회수
그러나 게임 속 세상에 들어오자마자 바이러스는 내 능력인 백신 모듈들을 흩어놓았다.  
내 미션은 명확하다.  
1. TrojanHorse.exe로 인해 분리된 **백신 모듈**들을 되찾고  
2. 백신 모듈의 힘인 **Virus Collector**로 남은 바이러스를 쓸어버리고 내 컴퓨터를 치료하는 것이다.

---



