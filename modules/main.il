.method static int32 main(string[])
{
		.entrypoint
		.maxstack 5
		.locals init ( char[][]	V_0, int32 V_1)

		ldarg.0
		ldlen
		conv.i4
		newarr char[]
		stloc.0
		ldc.i4.0
		stloc.1
		br IL_001f
	IL_0010:  ldloc.0
		ldloc.1
		ldarg.0
		ldloc.1
		ldelem.ref
		callvirt instance char[] string::ToCharArray()
		stelem.ref
		ldloc.1
		ldc.i4.1
		add
		stloc.1
	IL_001f:  ldloc.1
		ldarg.0
		ldlen
		conv.i4
		blt IL_0010
		ldloc.0
		call int32 DummyAssembly.DummyClass::start_func(char[][])
		ret
} // main