import cog

ret = ""
a = ""
b = ""

def setContext(pa, pb, r):
	a = pa
	b = pb
	ret = r

def dec(s):
	for i in ["bool", "char", "int", "float", "double"]:
		if s.find(i) != -1:
			return s+" "
	return s+" &"

def writeU(op, body):
	cog.outl("inline %s operator%s(%sa) {"%(ret,op,dec(a)))
	cog.outl(b)
	cog.outl("}")

def writeB(op, body):
	cog.outl("inline %s operator%s(%sa, %sb) {"%(ret,op,dec(a),dec(b)))
	cog.outl(body)
	cog.outl("}")

negateU = {'++':'--',  '--':'++'}
negateB = {'+': '-',   '-': '+'}
#given reciprocation/inversion, the following can be inferred:
recip =   {'*': '/',   '/': '*'}
#with ! on bool, the following may be converted
nots =    {'>=':'<',   '<=':'>', '<':'>=', '>':'<=', '==':'!=', '!=':'=='}
mutate = {'+': '+=', '-': '-=', '*': '*=', '/': '/='}
mutateR = {'+=': '+', '-=': '-', '*=': '*', '/=': '/'}

#todo
eqs = {'>': '>=', '<': '<='}
neqs = {'>=': '>', '<=': '<'}

def make(ops):
	for op in ops.keys:
		if op in negateB.keys and ops[op] == negateB[op]:
			apsnd([ops[op]], "-b")
		elif op in recip.keys and ops[op] == recip[op]:
			apsnd([ops[op]], "1/b")
		elif op in nots.keys and ops[op] == nots[op]:
			notOp([ops[op]])
		elif op in mutate.keys and ops[op] == mutate[op]:
			toMutator(mutate[op]);
		elif op in mutateR.keys and ops[op] == mutateR[op]:
			fromMutator(mutateR[op])
		elif op == ops[op]:
			commute([op])

#commutes given operators
def commute(ops):
	for op in ops:
		writeB(ret, op, "const "+b, "const "+a, "    return b %s a;"%op)

#applies a function to the first operand, allowing a different operator to be used in place
def apfst(ops, func):
	for op in ops.keys:
		writeB(ret, op, "const "+a, "const "+b, "    return %s %s b;"%(func,ops[op]))

#applies a function to the second operand, allowing a different operator to be used in place
def apsnd(ops, func):
	for op in ops.keys:
		writeB(ret, op, "const "+a, "const "+b, "    return a %s %s;"%(ops[op],func))

#++i -> i++, probably not useful for us...
def postfix(ops):
	for op in ops:
		writeB(ret, op, "const "+t, "int", "    %s temp = a;\n    %s a;\n    return temp;"%(t,op))

def toMutator(ops):
	for op in ops:
		writeB(ret + "&", op, a, "const"+b, "    return a = a %s b;" %op.rstrip("="))

def fromMutator(ops):
	for op in ops:
		writeB(ret, op, "const "+a, "const "+b, "    %s ret = a;\n    ret %s b;\n    return ret;"%(a,op + "="))

def notOp(ops):
	for op in ops.keys:
		writeB("bool", op, "const "+a, "const "+b, "    return !(a %s b);"%(ops[op]))
