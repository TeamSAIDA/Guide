# 중요한 사항

## 프로젝트 개요

* [Wiki : 봇 개발 튜터리얼 및 가이드 제공](https://github.com/TeamSAIDA/Guide/wiki)

* [Code : BasicBot 소스코드 제공](https://github.com/TeamSAIDA/Guide)

## BasicBot 개발 취지 및 특이사항

* 스타크래프트 봇 프로그램의 기본 기능 (미네랄 채취, 건물 짓기, 빌드오더 실행 등) 을 BasicBot 으로 개발하여 제공함

* [먼저 wiki 에서 개발환경 설정 방법, 튜터리얼 및 가이드를 읽어본 후 개발하는 것을 권장함](https://github.com/TeamSAIDA/Guide/wiki)

## BasicBot 설치방법

* master 브랜치를 Clone or download

# 상세 설명

## Code 폴더 설명

|폴더명|설명|
|----|----|
|C|BasicBot 및 TutorialBot 소스코드 C++ 버전|
|JAVA|BasicBot 및 TutorialBot 소스코드 JAVA 버전|
|docs\\C|BasicBot API Documentation - C++ 버전|
|docs\\JAVA|BasicBot API Documentation - JAVA 버전|
|Doxygen|BasicBot API Documentation 웹사이트 생성 자동화를 위한 Doxygen 설정 파일|

## BasicBot 디펜던시 및 권장 개발환경 ([wiki 에 상세 가이드](https://github.com/TeamSAIDA/Guide/wiki))

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
