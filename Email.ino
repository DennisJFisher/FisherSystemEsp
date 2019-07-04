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
        mail.setSubject("DennisJFisherRaspberry@gmail.com", "EMail Subject");
//        mail.addTo("to@example.com");
//        mail.addCC("cc@example.com");
//        mail.addBCC("bcc@example.com");
        
//        mail.addAttachment("test.txt", "This is content of attachment.");
        
        mail.setBody("This is an example e-mail.\nRegards Fisher");
        mail.setHTMLBody("This is an example html <b>e-mail<b/>.\n<u>Regards Fisher</u>");
        
        //mail.enableDebugMode();
        if (mail.send("smtp.gmail.com", 25, "DennisJFisherRaspberry@gmail.com", "salicia1.") == 0)
        //if (mail.send("smtp.gmail.com", 587, "DennisJFisherRaspberry@gmail.com", "salicia1.") == 0)
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
