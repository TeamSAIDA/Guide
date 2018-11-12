public class Main {
	
	/// 봇 프로그램을 실행합니다
	///
	/// Eclipse 메뉴 -> Run -> Run Configurations... 
	/// -> Arguments 탭 -> Working Directory 를 Other : C드라이브퍼StarCraft 로 변경하면
	/// 봇 프로그램이 기 생성되어있는 맵 파일 분석 캐시파일을 활용하게 되어서
	/// 봇 프로그램 실행 시 맵 파일 분석에 소요되는 딜레이를 줄일 수 있습니다.
	public static void main(String[] args) {
        new MyBotModule().run();
    }
}