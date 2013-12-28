/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

import java.io.*;
import java.net.*;

/**
 *
 * @author clary
 */
public class Comunicator extends Thread{

	private Socket socket = new Socket();
	private String ipAddress;

	private BufferedReader br;
	private BufferedWriter bw;

	private javax.swing.JTextArea textArea;

	public String ping()
	{
		return connectIfNot().sendSignal("PING");
	}

	public String[] getUsers()
	{
		return sendSignal("USERS").split(",");
	}

	public Comunicator login(String user)
	{
		if(!connectIfNot().sendSignal("LOGIN" + user).contains("OK"))throw new RuntimeException("Přihlášení se nezdařilo.");
		return this;
	}

	public Comunicator logout()
	{
		sendSignal("LOGOUT");
		closeConnection();
		return this;
	}

	public Comunicator sendToAll(String message)
	{

		sendSignal("ALL_MSG" + message + "\n");
		return this;
	}

	public Comunicator send(String message, String user)
	{
		String foo = sendSignal("PRIV_MSG" + user + message + "\n");
		if(foo.contains("ERR"))throw new RuntimeException("Zprávu se nepodařilo doručit.");
		return this;
	}

	public Comunicator closeConnection()
	{
		try{
			socket.close();
		} catch(IOException ioe) {
			throw new RuntimeException("Nelze se odpojit od serveru.");
		}
		return this;
	}

	public Comunicator setTextArea(javax.swing.JTextArea textArea)
	{
		this.textArea = textArea;
		return this;
	}

	public Comunicator setIpAddress(String ipAddress)
	{
		this.ipAddress = ipAddress;
		return this;
	}

	public void run()
	{
		try{
			while(true)
			{
				if(br.ready())textArea.append(br.readLine() + "\n");
				Thread.sleep(10);
			}
		}catch(IOException ioe){
			throw new RuntimeException("Nelze přijmout data.");
		}catch(InterruptedException ie){
			throw new RuntimeException("Bylo přerušeno čtecí vlákno.");
		}
	}

	private Comunicator connectIfNot()
	{
		try{
			if(!socket.isConnected())
			{
				socket.connect(new InetSocketAddress(ipAddress, 6200));
				br = new BufferedReader(new InputStreamReader(socket.getInputStream()));
				bw = new BufferedWriter(new OutputStreamWriter(socket.getOutputStream()));
			}
		} catch(IOException ioe) {
			throw new RuntimeException("Nelze se připojit na server.");
		}
		return this;
	}

	private String sendSignal(String signal)
	{
		try{
			bw.write(signal);
			bw.flush();
			return br.readLine();
		}catch(IOException ioe){
			throw new RuntimeException("Nelze poslat nebo přijmout data.");
		}
	}
}
