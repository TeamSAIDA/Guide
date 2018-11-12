# Change Log

## v1.1 Release (2017-07-19)

* BasicBot 신규 기능 추가

  * 타임아웃 패배, 자동 패배 체크 메소드 추가
  * 경기 결과 파일 Save / Load 및 로그파일 Save 예제 추가
  * onNukeDetect, onPlayerLeft, onSaveGame 이벤트 처리 가능하도록 메소드 추가

* BasicBot 중대한 버그 수정
  * 빌드 실행 유닛 (일꾼/건물) 결정 로직 버그 수정
  * 저그 뮤탈리스크 / 스커지 빌드 데드락 판단 로직 버그 수정
  * Refinery 건물 건설 위치 탐색 로직 버그 수정 및 속도 개선
  * 일꾼 탄생/파괴 등에 대한 업데이트 로직 버그 수정

* BasicBot 기타 수정사항
  * 멀티 기지간 일꾼 숫자 리밸런싱 조건값 수정
  * 파일경로 기본값 수정
  * getUnitAndUnitInfoMap 메소드에 대해 const 제거 (C++만 해당)
  * appendTextToFile 등 메소드를 static 으로 수정 (JAVA 만 해당)

* BuildServerCode 예시적으로 수록

* TutorialLevel5Bot 에 BasicBot 수정사항 반영

## v1.0 Release (2017-06-16)

* fix minor bug : BuildManager.java 의 update, checkBuildOrderQueueDeadlockAndAndFixIt 함수 수정

  * 변경 사유 : 특정 unitID 혹은 seedLocation 를 지정해서 빌드오더를 입력하는 경우에 대해 잘 처리하도록 수정

* change Config default value : drawMapGrid 등 일부 값을 false 로 변경

* remove SeedPositionStrategy.SecondExpansionLocation : 참가자의 전략적 선택에 관한것이라서 제거

## v0.96 beta (2017-06-15)

* fix critical bug : CommandUtil.java 의 attackMove, move, rightClick 함수 버그 수정

* fix critical bug : CommandUtil.java 의 IsValidUnit 함수 버그 수정

## v0.95 beta (2017-06-09)

* change C++ encoding : UTF-8 without signature 에서 UTF-8 with signature 로 변경

  * 변경 사유 : Visual Studio 2013 에서 UTF-8 without signature 는 컴파일 에러 발생

* fix minor error : GameCommander.cpp 수정

  * MapTools::Instance().draw() 문장 잘못 삽입되어있던것 삭제

## v0.94 beta (2017-06-02)

* change C++/JAVA encoding : MS-949 에서 UTF-8 without signature 로 변경

  * 변경 사유 : GitHub 및 일반 텍스트에디터에서도 잘 보이도록 변경

## v0.93 beta (2017-05-31)

* fix miner bugs

## v0.92 beta (2017-05-31)

* README 수정

* docs 재생성

## v0.91 beta (2017-05-30)

* MIT License 라이센스 표기 및 저작자 표기

* fix critical error : UnitData.removeBadUnits() 등

* change code : 플레이어 이탈 시 작동 중지하도록 수정

## v0.90 beta (2017-05-29)

* 알고리즘 경진대회 BasicBot 소스코드 사전 공개 및 오픈 베타 테스트 시작
