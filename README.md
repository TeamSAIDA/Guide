# 중요한 사항

## 프로젝트 개요

* 2018 삼성SDS 알고리즘 경진대회를 위한 프로젝트

* [Wiki : 경진대회 개발환경 설정 방법, 봇 개발 튜터리얼 및 가이드 제공](https://github.com/TeamSAIDA/Guide/wiki)

* [Code : 경진대회 봇 프로그램 개발의 시작점으로 사용할 BasicBot 소스코드 제공](https://github.com/TeamSAIDA/Guide)

## BasicBot 개발 취지 및 특이사항

* 경진대회 결과물의 수준을 상향화 하기 위해 스타크래프트 봇 프로그램의 기본 기능 (미네랄 채취, 건물 짓기, 빌드오더 실행 등) 을 BasicBot 으로 개발하여 제공함

* 경진대회 참가자들은 BasicBot 을 자유롭게 수정하여 개발을 수행한 후 소스코드를 제출하면 됨

  * 단, Main, MyBotModule, UXManager 는 제출하더라도 원활한 대회 진행을 위해 봇 컴파일에 반영하지않음

* [먼저 wiki 에서 개발환경 설정 방법, 튜터리얼 및 가이드를 읽어본 후 개발하는 것을 권장함](https://github.com/TeamSAIDA/Guide/wiki)

## BasicBot 설치방법

* master 브랜치를 Clone or download

## BasicBot Version

* **2017-07-19 : v1.1 Release (실제 알고리즘 경진대회에서 사용)**

* **2017-06-16 : v1.0 Release (실제 알고리즘 경진대회에서 사용)**

* **2017-05-29 : v0.9 beta Release (프리뷰 및 오픈 테스트를 위해 공개)**

* [버그를 발견하여 알려주시거나 이슈를 제기하여주시면 감사의 선물을 드리겠습니다](https://github.com/SamsungSDS-Contest/Guide/issues)


## BasicBot API Documentation

* BasicBot API Documentation 웹사이트 : [https://samsungsds-contest.github.io/Guide/](https://samsungsds-contest.github.io/Guide/)

# 상세 설명

## Code 폴더 설명

|폴더명|설명|
|----|----|
|C|BasicBot 및 TutorialBot 소스코드 C++ 버전|
|JAVA|BasicBot 및 TutorialBot 소스코드 JAVA 버전|
|docs\\C|BasicBot API Documentation - C++ 버전|
|docs\\JAVA|BasicBot API Documentation - JAVA 버전|
|Doxygen|BasicBot API Documentation 웹사이트 생성 자동화를 위한 Doxygen 설정 파일|

## BasicBot 디펜던시 및 권장 개발환경 ([wiki 에 상세 가이드](https://github.com/SamsungSDS-Contest/Guide/wiki))

* StarCraft : Brood War 1.16.1.1

* C++

  * BWAPI 4.1.2

  * BWTA 2.2

  * Visual Studio 2013

  * 파일 인코딩 : UTF-8 with signature (BOM)

* JAVA

  * BWMirror 2.5 (BWAPI 4.1.2 및 BWTA 2 가 포함되어 있음)

  * JDK 32bit

  * Eclipse or IntelliJ

  * 파일 인코딩 : UTF-8 without signature (BOM)

## BasicBot 개발 History

* 삼성SDS 알고리즘경진대회 준비 T/F에서 오픈소스 [uAlbertaBot](https://github.com/davechurchill/ualbertabot), [Atlantis](https://github.com/Ravaelles/Atlantis), [BWSAL](https://github.com/Fobbah/bwsal) 등을 참고하여 BasicBot (C++ 버전 및 JAVA 버전) 을 개발하여 배포함

## BasicBot API Documentation 생성 툴

* [Doxygen 1.8.8](http://www.doxygen.org/index.html) 을 사용하여 BasicBot API Documentation 웹사이트를 생성하였음
