import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class Common {

	// BasicBot 1.1 Patch Start ////////////////////////////////////////////////
	// appendTextToFile 등 메소드를 static 으로 수정

	/// 로그 유틸
	public static void appendTextToFile(final String logFile, final String msg)
	{
		try {
			BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(new File(logFile), true))  ;
			bos.write(msg.getBytes());
			bos.flush();
			bos.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/// 로그 유틸
	public static void overwriteToFile(final String logFile, final String msg)
	{
		try {
			BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(new File(logFile)))  ;
			bos.write(msg.getBytes());
			bos.flush();
			bos.close();
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/// 파일 유틸 - 텍스트 파일을 읽어들인다
	public static String readFile(final String filename)
	{
		BufferedInputStream bis;
		StringBuilder sb = null;
		try {
			bis = new BufferedInputStream(new FileInputStream(new File(filename)));
	        sb = new StringBuilder();

	        while (bis.available() > 0) {
	            sb.append((char) bis.read());
	        }
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}

		return sb.toString();
	}
	
	// BasicBot 1.1 Patch End //////////////////////////////////////////////////
	
}