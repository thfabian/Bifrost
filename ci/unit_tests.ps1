Param(
    [String]$path
)

if(-not($path)) { Throw "-path is required" }
$exitCode = 0;

Write-Host "Running tests from $path ...";
Foreach($i in "bifrost-core", "bifrost-api") {
    Write-Host "Running test $path\test-$i.exe ...";
    $outFile = "$path\$i-output.txt";
    $errFile = "$path\$i-err.txt";
    Remove-Item –path $outFile -ErrorAction Ignore;
    Remove-Item –path $errFile -ErrorAction Ignore;

    # Run the tests and print stdout/stderr
    $procArgs = @{
        FilePath = "$path\test-$i.exe"
        ArgumentList = "--gtest_output=xml:$i-gtest-result.xml"
        Wait = $true
        NoNewWindow = $true
        RedirectStandardOutput = $outFile
        RedirectStandardError = $errFile
        PassThru = $true
        WorkingDirectory = $path
    }

    $proc = Start-Process @procArgs;
    $proc.WaitForExit()

    Get-Content -Path $outFile;
    Get-Content -Path $errFile;

    $testExitCode = $proc.ExitCode;
    Write-Host "Test completed with exit code: $testExitCode";
    If($testExitCode -ne 0) {
        $exitCode = 1;
    }

    # Upload the gtest output to appveyor
    If (Test-Path $path\$i-gtest-result.xml) {
        If(Test-Path env:PPVEYOR_JOB_ID) {
            (New-Object System.Net.WebClient).UploadFile("https://ci.appveyor.com/api/testresults/junit/$($env:APPVEYOR_JOB_ID)", (Resolve-Path $path\$i-gtest-result.xml));
        }
    }
}
Write-Host "Tests completed with exit code: $exitCode";
exit $exitCode;