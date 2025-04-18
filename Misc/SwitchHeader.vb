Imports System
Imports EnvDTE
Imports EnvDTE80
Imports EnvDTE90
Imports System.Diagnostics

Public Module Module1

    Sub SwitchHeader()
        Dim FileName, NewFileName As String
        Dim Extensions() As String = {".h", ".hpp", ".cpp", ".c"}
        Dim I, I1 As Integer
        Dim NewWin As Window

        If IsNothing(DTE.ActiveDocument) Then
            Exit Sub
        End If

        FileName = DTE.ActiveDocument.FullName.ToLower
        I = Extensions.GetLowerBound(0)
        While I <= Extensions.GetUpperBound(0) AndAlso Not FileName.EndsWith(Extensions(I))
            I = I + 1
        End While
        If I > Extensions.GetUpperBound(0) Then
            Exit Sub
        End If
        FileName = FileName.Remove(FileName.Length - Extensions(I).Length)
        I1 = I
        Do
            I1 = (I1 - Extensions.GetLowerBound(0) + 1) Mod Extensions.GetLength(0) + Extensions.GetLowerBound(0)
            NewFileName = FileName + Extensions(I1)
            On Error Resume Next
            NewWin = DTE.ItemOperations.OpenFile(NewFileName, Constants.vsViewKindTextView)
            If Not IsNothing(NewWin) Then
                NewWin.Activate()
                Exit Do
            End If
        Loop While I1 <> I
    End Sub

End Module

