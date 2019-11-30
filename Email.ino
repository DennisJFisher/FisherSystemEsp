#if 0
#include <ESPMail.h>

ESPMail mail;

bool SendEmail = false;

void Email_Setup()
{
  mail.begin();
}

void Email_Loop()
{
    if (SendEmail)
    {        
        mail.setSubject((char *)"DennisJFisherRaspberry@gmail.com", (char *)"EMail Subject");
//        mail.addTo("to@example.com");
//        mail.addCC("cc@example.com");
//        mail.addBCC("bcc@example.com");
        
//        mail.addAttachment("test.txt", "This is content of attachment.");
        
        mail.setBody((char *)"This is an example e-mail.\nRegards Fisher");
        mail.setHTMLBody((char *)"This is an example html <b>e-mail<b/>.\n<u>Regards Fisher</u>");
        
        //mail.enableDebugMode();
        if (mail.send((char *)"sWmtp.gmail.com", 25, (char *)"DennisJFisherRaspberry@gmail.com", (char *)"salicia1.") == 0)
        //if (mail.send((char *)"smtp.gmail.com", 587, (char *)"DennisJFisherRaspberry@gmail.com", (char *)"salicia1.") == 0)
        {
            Serial.println("Mail send OK");
        }
        else
        {
            Serial.println("Mail send NOT OK");
        }
      
        SendEmail = false;
    }
}
#endif
