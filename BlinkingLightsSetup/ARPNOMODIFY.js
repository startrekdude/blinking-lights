var installer = WScript.CreateObject("WindowsInstaller.Installer");
var database = installer.OpenDatabase("BlinkingLightsSetup.msi", 1);

var view = database.OpenView("INSERT INTO Property(Property.Property, Property.Value) VALUES('ARPNOMODIFY', '1')");
view.Execute();

database.Commit();