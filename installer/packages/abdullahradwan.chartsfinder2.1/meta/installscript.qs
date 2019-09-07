function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") 
	{
        component.addOperation("CreateShortcut", "@TargetDir@/ChartsFinder2.exe", "@StartMenuDir@/Charts Finder 2.lnk",
            "workingDirectory=@TargetDir@");
        component.addOperation("CreateShortcut", "@TargetDir@/ChartsFinder2.exe", "@DesktopDir@/Charts Finder 2.lnk",
            "workingDirectory=@TargetDir@");
    } else 
	{
		component.addElevatedOperation("Execute", "chmod", "-R", "777", "@TargetDir@");
		component.addElevatedOperation("CreateDesktopEntry", 
                                  "/usr/share/applications/ChartsFinder2.desktop", 
                                  'Type=Application\nTerminal=false\nExec="@TargetDir@/ChartsFinder2"\nName=Charts Finder 2\nIcon=@TargetDir@/icon.svg\nCategories=Utility;');
	}
}
