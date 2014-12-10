<%@page import="java.io.PrintWriter"%>
<%@page import="java.io.ByteArrayOutputStream"%>
<%@page import="confdb.converter.OnlineConverter"%>
<%@ page contentType="text/plain" %>
<%
  out.clearBuffer();
  try {
	int configKey = Integer.parseInt( request.getParameter( "configKey" ) );
	OnlineConverter converter = OnlineConverter.getConverter();
	String confStr = converter.getEpConfigString( configKey );
	if ( confStr == null )
	  out.print( "ERROR!\nconfig " + configKey + " not found!" );
	else
	  out.print( confStr );
  } catch ( Exception e) {
	  ByteArrayOutputStream buffer = new ByteArrayOutputStream();
	  PrintWriter writer = new PrintWriter( buffer );
	  Throwable cause = e.getCause();
	  if ( cause != null )
	  {
		  writer.println( e );
		  cause.printStackTrace( writer );
	  }
	  else
		  e.printStackTrace( writer );
	  writer.close();
	  out.println( "ERROR!\n" + buffer.toString() );
  }
%>


