package br.org.scadabr.dnp34j.samples;

import java.awt.Button;
import java.awt.Color;
import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.JTextPane;

import com.serotonin.db.spring.KeyGeneratingPreparedStatementCreator;

public class TestServer extends JFrame implements Runnable{
	
	static Button b1;
	static Button b2;
	static Button b3;
	static String sendText;
	JPanel jp1,jp2,jp3;
	static JTextArea jta;
	static JTextPane jtp;
	static JScrollPane jslp;
	static JTextField jtf;
	static boolean flg=false,flg_end=false;
	
	static byte[] buf=new byte[1024];
	static char[] by ={0x05,0x64,0x05,0xc0,0x00,0x01,0x00,0x02,0x5c,0x95};//,0x65,0x96,0x85,0x86,0x87,0x88,0x89,0x91,0x92,0x93,0x94,0x95,0x98,0x99,0x96,0x97,0x96,0x96};
	//056405c0022010019c9b
	//056405c0022010019c9b
	
	static Socket s;
	static ServerSocket ss;
	static DataInputStream dis;
	static DataOutputStream dos;
	static byte r;
	static Thread t1;

	public TestServer(){
		b1=new Button("打开连接");
		b2=new Button("清屏");
		b3=new Button("发送");
		jtf=new JTextField("",50);
		jta=new JTextArea("",19,50);
		jtp=new JTextPane();
		jslp=new JScrollPane(jta);
		jta.setText("");
		jp1=new JPanel(new FlowLayout());
		jp2=new JPanel(new FlowLayout());
		jp3=new JPanel(new FlowLayout());
		jp1.add(b1,"West");
		jp1.add(b3,"Center");
		jp1.add(b2,"East");
		jp2.add(jslp);
		jp3.add(jtf);
		b1.setBackground(Color.green);
		/**************************/
		this.setTitle("Master");
		this.setBounds(300, 200, 600, 450);
		this.add(jp1,"South");
		this.add(jp3,"Center");
		this.add(jp2,"North");
		this.setDefaultCloseOperation(EXIT_ON_CLOSE);
		this.setVisible(true);
		/********************************/
	}
	private static void init() {
		/**************/
		jtf.addKeyListener(new KeyListener() {
			@Override
			public void keyTyped(KeyEvent e) {
				
			}
			@Override
			public void keyReleased(KeyEvent e) {
				
			}
			@Override
			public void keyPressed(KeyEvent e) {
				if('\n'==e.getKeyChar()){
					send();
				}
			}
		});
		/****************/
		b2.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				jta.setText("");
			}
		});
		/***************************/
		b3.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				send();
			}
		});
		/***********************/
		b1.addActionListener(new ActionListener() {
			@Override
			public void actionPerformed(ActionEvent e) {
				try {
					if(!flg){
						b1.setBackground(Color.red);
						b1.setLabel("关闭连接");
						s=new Socket();
						ss=new ServerSocket(20000);
						s = ss.accept();
						flg_end=true;
						jta.append("有用户连接上来了！可以开始发送了 ！");
						dis=new DataInputStream(s.getInputStream());
						dos=new DataOutputStream(s.getOutputStream());
						flg=true;
						
					}else{
						interupt();
					}
				} catch (IOException ee) {
					ee.printStackTrace();
				}
			}
		});
		
	}
	private static void interupt() throws IOException {
		b1.setBackground(Color.GREEN);
		b1.setLabel("打开连接");
		if(flg_end){
			dos.write(254);				//发送连个结束
			dos.write(254);	
		}
		
		dis.close();
		dos.close();
		ss.close();
		s.close();
		flg=false;
	}
	private static void send() {
		sendText = jtf.getText();
		
		//处理输入的报文
		sendText=sendText.replace(" ", "");
		sendText=sendText.replace("0x", "");
		sendText=sendText.replace(",", "");
		sendText=sendText.replace("\t", "");
		sendText=sendText.replace("\r", "");
		
		if(sendText.equals(""))	return;			//空串就return

		jta.append('\n'+"<---"+sendText);
		jtf.setText("");
		
		int len = sendText.length();
		if(len%2!=0){
			jta.append("\t Error:字符长度不是偶数");
			return;
		}
		System.out.println("发送字符为 :"+sendText);
		int j=0;
		for(int i=0;i<len;i+=2){
			String sub =sendText.substring(i,2+i);
			try{
				buf[j++]=(byte) Integer.parseInt(sub, 16);
			}catch(Exception e){
				jta.append("\t Error:报文错误，不能解析");
				return;
			}
		}
		System.out.print("解析的报文为:");
		for(int i=0;i<j;i++){
			System.out.print(buf[i]);
		}
		System.out.println();
		/***************开始发送************/
		try {
			dos.write(sendText.length()/2/256);				//把长度发过去
			dos.write(sendText.length()/2%256);				//把长度发过去
		} catch (IOException e1) {
			e1.printStackTrace();
		}
		for(int i=0;i<j;i++){
			try {
				dos.write(buf[i]);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
		jta.append("\t Success:"+sendText.length()/2);
		jta.append("\r\n--->");
	}
	
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		TestServer ts= new TestServer();
		t1=new Thread(ts);
		t1.start();
		init();
	}
	@Override
	public void run() {
		while(true){
			while(flg){
				try {
					String hs = Integer.toHexString(dis.readByte());
					if(hs.length()<2) hs="0"+hs;	
					jta.append(hs);				
					Thread.sleep(10);
				} catch (InterruptedException e) {
					flg_end=false;
					jta.append("\r\n从端已经关闭连接1\r\n");
					try {
						interupt();
					} catch (IOException e1) {
						e1.printStackTrace();
					}
					break;
					//e.printStackTrace();
				} catch (IOException e) {
					flg_end=false;
					jta.append("\r\n从端已经关闭连接2\r\n");
					try {
						interupt();
					} catch (IOException e1) {
						e1.printStackTrace();
					}
					break;
					//e.printStackTrace();
				}
			}
		}
	}
}

