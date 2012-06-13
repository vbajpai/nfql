import ply.lex as lex
import ply.yacc as yacc
from statement import *
from ply.yacc import SyntaxError
import netaddr


class Lexer(object):
    def __init__(self,**kwargs):
        self.lexer = lex.lex(module=self, **kwargs)

    reserved = {
                'splitter' : 'splitterKeyword',
                'groupfilter' : 'groupFilterKeyword',
                'filter' : 'filterKeyword',
                'grouper' : 'grouperKeyword',
                'module' : 'moduleKeyword',
                'merger' : 'mergerKeyword',
                'export' : 'exportKeyword',
                'ungrouper' : 'ungrouperKeyword',
                'branches' : 'branchesKeyword',
                'branch' : 'branchKeyword',
                'aggregate' : 'aggregateKeyword',
                'as' : 'asKeyword',
                'min' : 'minKeyword',
                'max' : 'maxKeyword',
                'avg' : 'avgKeyword',
                'sum' : 'sumKeyword',
                'count' : 'countKeyword',
                'union' : 'unionKeyword',
                'in' : 'inKeyword',
                'notin' : 'notinKeyword',
                'OR' : 'ORKeyword',
                'NOT' : 'NOTKeyword',
                'bitOR': 'bitORKeyword',
                'bitAND' : 'bitANDKeyword',
                'm' : 'mKeyword',
                'mi' : 'miKeyword',
                'o' : 'oKeyword',
                'oi' : 'oiKeyword',
                's' : 'sKeyword',
                'si' : 'siKeyword',
                'd' : 'dKeyword',
                'di' : 'diKeyword',
                'f' : 'fKeyword',
                'fi' : 'fiKeyword',
                'eq' : 'eqKeyword',     # prevent clash with = for match rules
                'delta': 'deltaKeyword',
                'rdelta' : 'rdeltaKeyword',
                'ms' : 'msKeyword'
                }


    def t_LTEQ(self, t):
        r'<='
        t.value = 'LTEQ'
        return t

    def t_GTEQ(self, t):
        r'>='
        t.value = 'GTEQ'
        return t

    def t_ML(self, t):
        r'<<'
        t.value = 'ML'
        return t

    def t_MG(self, t):
        r'>>'
        t.value = 'MG'
        return t
    
    def t_LT(self, t):
        r'<'
        t.value = 'LT'
        return t

    def t_EQ(self, t):
        r'='
        t.value = 'EQ'
        return t

    def t_GT(self, t):
        r'>'
        t.value = 'GT'
        return t


    tokens = ['id', 'LT', 'EQ', 'GT',
              'LTEQ', 'GTEQ', 'ML', 'MG',
              'MAC', 'IPv4', 'IPv6',
              'int', 'float', 'hex',
              'string'] + list(reserved.values())

    t_ignore  = ' \t'
    t_ignore_comment = r'\#.*'

    literals = "+-*/(){},."

    def t_string(self, t):
        r'"[^"\\\r\n]*(?:\\.[^"\\\r\n]*)*"'
        t.value = Arg("string", t.value[1:-1].replace("\\",''), t.value)
        return t

    def t_IPv4(self, t):
        r'\b\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}'
        #the regex does include invalid IPs but they are
        #checked later during conversion
        try:
            t.value =Arg("addr_IPv4", int(netaddr.IP(t.value)), t.value)
            return t
        except netaddr.AddrFormatError:
            message = 'Bad IPv4 format %s at line %s' %(t.value,  
                                                        t.lexer.lineno)
            raise SyntaxError(message)

    def t_MAC(self, t):
        r'([a-fA-F0-9]{2}[:\-]){5}[a-fA-F0-9]{2}'
        try:
            t.value = Arg("addr_MAC", int(netaddr.EUI(t.value)), t.value)
            return t
        except netaddr.AddrFormatError:
            message = 'Bad MAC format %s at line %s' %(t.value,  
                                                        t.lexer.lineno)
            raise SyntaxError(message)

    def t_IPv6(self, t):
        r'(::[0-9a-f]{1,4}[0-9a-f:]*)|([0-9a-f]:[0-9a-f:]*)'
        # the regular expression is very genral, so this rule should be
        # after the other address rules.
        try:
            t.value = Arg("addr_IPv6", int(netaddr.IP(t.value)), t.value)
            return t
        except netaddr.AddrFormatError:
            message = 'Bad IPv6 format %s at line %s' %(t.value,  
                                                        t.lexer.lineno)
            raise SyntaxError(message)

    def t_float(self, t):
        '[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?'
        t.value = Arg("float", float(t.value), t.value)
        return t

    def t_hex(self, t):
        r'0[xX][0-9a-fA-F]+'
        t.value = Arg("int", int(t.value, 0), t.value)
        return t

    def t_int(self, t):
        r'\d+'
        t.value = Arg("int", int(t.value), t.value)
        return t

    def t_id(self, t):
        r'[a-zA-Z_][a-zA-Z_0-9]*'
        # matches also keywords, so be careful
        t.type = self.reserved.get(t.value,'id')    # Check for reserved words
        return t

    def t_newline(self, t):
        r'\n+'
        t.lexer.lineno += len(t.value)

    # Error handling rule
    def t_error(self,t):
        msg = "Illegal character '%s'" % t.value[0]
        raise SyntaxError(msg)

    # Test it output
    def test(self,data):
        self.lexer.input(data)
        while True:
            tok = self.lexer.token()
            if not tok: break
            print tok

class Parser(object):
    # the tokens from the lexer class:
    tokens = Lexer.tokens

    def __init__(self):
        self.filters = []
        self.groupers = []
        self.splitter = None
        self.group_filters = []
        self.mergers = []
        self.branch_names = set()
        self.ungroupers = []
        self.branches = []
        self.input = None
        self.outputs = []
        self.names = {}
        self.lexer = Lexer().lexer
        self.parser = yacc.yacc(module=self)

    def p_file(self,p):
        '''file : pipeline_stage_1n'''
#        for k, v in self.names.iteritems():
#            print k, v

    def p_pipeline_stage_1n(self,p):
        'pipeline_stage_1n : pipeline_stage pipeline_stage_1n'
        # add a name mapping:
        try:
            # branch statements dont have names 
            # so we skip them with try/except
            self.names[p[1].name] = p[1]
        except AttributeError:
            pass

    def p_pipeline_stage_end(self,p):
        'pipeline_stage_1n :'

    def p_pipeline_stage(self,p):
        '''
        pipeline_stage : splitter
                       | filter
                       | composite_filter
                       | branch
                       | ungrouper
                       | grouper
                       | group_filter
                       | merger
        '''
        
        p[0] = p[1]

    def p_splitter(self,p):
        '''
        splitter : splitterKeyword id '{' '}'
        '''
        p[0] = Splitter(p[2], p.lineno(2))
        if self.splitter != None:
            raise SyntaxError(
            "More than one splitter defined in file at line %s",p.lineno(2))

        self.splitter = p[0]

    def p_filter(self,p):
        '''
        filter : filterKeyword id '{' filter_rule_1n '}'
        '''
        # Note that p[4] is a list of lists of rules.
        # If the list has one element the rule is simple.
        # If the rule has more than one element, the 
        # rule is OR-ed of all the rules in the list
        p[0] = Filter(p[2], p.lineno(2), p[4])
        self.filters.append(p[0])
        
    
    def p_composite_filter(self, p):
        '''
        composite_filter : filterKeyword id '{' filter_ref_rule_1n '}'
        '''
        # Note that p[4] is a list of lists of rules.
        # If the list has one element the rule is simple.
        # If the rule has more than one element, the 
        # rule is OR-ed of all the rules in the list
        p[0] = Filter(p[2], p.lineno(2), p[4])
        self.filters.append(p[0])

    def p_group_filter(self, p):
        '''
        group_filter : groupFilterKeyword id '{' filter_rule_1n '}'
        '''
        # Note that p[4] is a list of lists of rules.
        # If the list has one element the rule is simple.
        # If the rule has more than one element, the 
        # rule is OR-ed of all the rules in the list
        p[0] = Filter(p[2], p.lineno(2), p[4])
        self.group_filters.append(p[0])

    def p_filter_rule_1n(self,p):
        'filter_rule_1n : filter_rule filter_rule_1n'
        p[2].extend([p[1]])
        p[0] = p[2]

    def p_filter_rule_0(self,p):
        'filter_rule_1n :'
        p[0] = []

    def p_filter_rule(self,p):
        '''
        filter_rule : or_rule
        '''
        p[0] = p[1]

    def p_filter_ref_rule_1n(self,p):
        'filter_ref_rule_1n : filter_ref_rule filter_ref_rule_1n'
        p[2].extend([p[1]])
        p[0] = p[2]

    def p_filter_ref_rule_0(self,p):
        'filter_ref_rule_1n : filter_ref_rule'
        p[0] = [p[1]]

    def p_filter_ref_rule(self,p):
        '''
        filter_ref_rule : or_id
        '''
        p[0] = p[1]

    def p_or_id(self, p):
        'or_id : not_id opt_or_id'
        p[1].extend(p[2])
        p[0] = p[1]

    def p_opt_or_id(self, p):
        '''
        opt_or_id : ORKeyword not_id opt_or_id
        '''
        p[2].extend(p[3])
        p[0] = p[2]

    def p_opt_or_id_end(self, p):
        'opt_or_id :'
        p[0] = []

    def p_not_id(self, p):
        '''
        not_id : NOTKeyword id
               | id
        '''
        try:
            p[0] = [FilterRef(p[2], p.lineno(2), True)]
        except IndexError:
            p[0] = [FilterRef(p[1], p.lineno(1))]

    def p_or_optrule(self,p):
        'or_rule : rule_or_not opt_rule'
        if len(p[2]) > 0:
            ors = [p[1]]
            ors.extend(p[2])
            p[0] = ors
        else:
            p[0] = [p[1]]

    def p_or_rule(self, p):
        'opt_rule : ORKeyword rule_or_not opt_rule'
        res = [p[2]]
        res.extend(p[3])
        p[0] = res

    def p_term_opt_rule(self,p):
        'opt_rule :'
        p[0] = []

    def p_rule_or_not(self, p):
        '''
        rule_or_not : rule
                    | NOTKeyword rule
        '''
        try:
            p[2].NOT = True
            p[0] = p[2]
        except IndexError:
            p[0] = p[1]

    def p_rule(self,p):
        '''
        rule : infix_rule
             | prefix_rule
        '''
        p[0] = p[1]

    def p_infix_rule(self,p):
        'infix_rule : arg op arg'
        p[1].extend(p[3]) # concatenate args to get [arg, arg]
        # for some unknown reason p.lineno(2) does not work in this production
        # so p[2] is (op, lineno)
        p[0] = Rule(p[2][0], p[2][1], p[1]) # (op, line, args)

    def p_op(self, p):
        '''
        op : EQ
           | LT
           | GT
           | LTEQ
           | GTEQ
           | ML
           | MG
           | inKeyword
           | notinKeyword 
        '''
        p[0] = (p[1], p.lineno(1))
    
    def p_rule_prefix(self,p):
        '''
        prefix_rule : id '(' args ')'
                    | bitANDKeyword '(' args ')'
                    | bitORKeyword '(' args ')'
        '''
        p[0] = Rule(p[1], p.lineno(1), p[3])

    def p_args(self,p):
        '''
        args : arg ',' args
        '''
        p[0] = p[1]
        p[0].extend(p[3]) # concatenate the rest of the args to arg

    def p_args_more(self,p):
        'args : arg'
        p[0] = p[1]

    def p_no_args(self, p):
        'args :'
        p[0] = []

    def p_arg(self, p):
        '''
        arg : id
            | IPv4
            | IPv6
            | CIDR
            | MAC
            | int
            | float
            | hex
            | prefix_rule
            | string
        '''
        if type(p[1]) is type("string"):
            p[1] = Field(p[1])
        p[0] = [p[1]] # list of one element for easy [].extend later
        
    def p_cidr(self, p):
        '''
        CIDR : IPv4 '/' int
             | IPv6 '/' int
        '''
        p[0] = Rule('cidr_mask', p[1], p[3])

    def p_start_branch(self, p):
        '''
        branch : id arrow mid_branch
        '''
        br = [BranchNode(p[1], p.lineno(1))]
        br.extend(p[3])
        p[0] = br
        self.branches.append(p[0])

    def p_input_branch(self, p):
        '''
        branch : string arrow mid_branch
        '''
        if self.input != None:
            raise SyntaxError("More than one input defined in file at line %s",
                              p.lineno(1))
        self.input = Input(p[1].value, p.lineno(1))
        br = [self.input]
        br.extend(p[3])
        p[0] = br
        self.branches.append(p[0])

    def p_split_branch(self, p):
        '''
        branch : id branchKeyword mid_branch
        '''
        br = [BranchNode(p[1], p.lineno(1))]
        p[3][0] = Branch(p[3][0].name, p[3][0].line)
        br.extend(p[3])
        p[0] = br
        self.branches.append(p[0])

    def p_mid_branch(self, p):
        '''
        mid_branch : id arrow mid_branch
        '''
        br = [BranchNode(p[1], p.lineno(1))]
        br.extend(p[3])
        p[0] = br


    def p_mid_branch_terminate(self, p):
        '''
        mid_branch : end_branch
        '''
        p[0] = p[1]

    def p_end_branch(self, p):
        'end_branch : id'
        p[0] = [BranchNode(p[1], p.lineno(1))]
        
    def p_output_branch(self, p):
        'end_branch : string'
        out = Output(p[1].value, p.lineno(1))
        self.outputs.append(out)
        p[0] = [out]


    def p_arrow(self, p):
        """arrow : "-" GT"""
        pass

    def p_ungrouper(self, p):
        '''
        ungrouper : ungrouperKeyword id '{' '}'
        '''
        p[0] = Ungrouper(p[2], p.lineno(2))
        self.ungroupers.append(p[0])

    def p_grouper(self, p):
        "grouper : grouperKeyword id '{' module1_n aggregate '}'"
        p[0] = Grouper(p[2], p.lineno(2), p[4], p[5])
        # insert aggregation of record ids (needed for ungrouping later)
        p[0].aggr.insert(0,(Rule('union', p.lineno(2), [Field('rec_id'),
                                                        'records'])))
        p[0].aggr.insert(0,(Rule('min', p.lineno(2), [Field('stime'),
                                                        'stime'])))
        p[0].aggr.insert(0,(Rule('max', p.lineno(2), [Field('etime'),
                                                        'etime'])))
        self.groupers.append(p[0])

    def p_module1_n(self, p):
        'module1_n : module module1_n'
        p[1].extend(p[2])
        p[0] = p[1]

    def p_module0(self, p):
        'module1_n :'
        p[0] = []

    def p_module(self, p):
        "module : moduleKeyword id '{' grouper_rule1_n '}'"
        p[0] = [Module(p[2], p.lineno(2), p[4])]

    def p_grouper_rule1_n(self, p):
        'grouper_rule1_n : grouper_rule grouper_rule1_n'
        p[1].extend(p[2])
        p[0] = p[1]

    def p_grouper_rule0(self, p):
        'grouper_rule1_n :'
        p[0] = []

    def p_grouper_rule(self, p):
        'grouper_rule : id grouper_op id'
        p[0] = [[GrouperRule(p[2], p.lineno(2), [Field(p[1]), Field(p[3]),
                                           None, False])]]

    def p_grouper_rule_delta(self, p):
        '''
        grouper_rule : id grouper_op id deltaKeyword delta_arg
        '''
        p[0] = [[GrouperRule(p[2], p.lineno(2), [Field(p[1]), Field(p[3]),
                                           p[5], False])]]

    def p_grouper_rule_rel_delta(self, p):
        '''
        grouper_rule : id grouper_op id rdeltaKeyword delta_arg
        '''
        p[0] = [[GrouperRule(p[2], p.lineno(2), [Field(p[1]), Field(p[3]),
                                           p[5], True])]]

    def p_grouper_op(self, p):
        '''
        grouper_op : EQ
                   | LT
                   | GT
                   | GTEQ
                   | LTEQ
        '''
        p[0] = p[1]
    def p_delta_arg(self, p):
        '''
        delta_arg : time
                  | int
        '''
        p[0] = p[1]

    def p_time(self, p):
        '''
        time : int sKeyword
             | int msKeyword
             | int minKeyword
        '''
        # the number should be in ms:
        if p[2] == 's':
            p[1].value = p[1].value * 1000
        if p[2] == 'min':
            p[1].value = p[1].value  * 60 * 1000
        p[0] = p[1]

    def p_aggregate(self, p):
        'aggregate : aggregateKeyword aggr1_n'
        for aggr in p[2]:
            if aggr.line == 0:
                aggr.line = p.lineno(1)
        p[0] = p[2]

    def p_aggr1_n(self, p):
        'aggr1_n : aggr opt_aggr'
        p[1].extend(p[2])
        p[0] = p[1]

    def p_opt_aggr(self, p):
        "opt_aggr : ',' aggr opt_aggr"
        p[2].extend(p[3])
        p[0] = p[2]

    def p_opt_aggr_end(self, p):
        'opt_aggr :'
        p[0] = []

    def p_aggr(self, p):
        "aggr : aggr_op '(' id_or_qid ')' asKeyword id"
        args = [Field(p[3]), p[6]] # [id_or_qid, id, aggr_op]
        p[0] = [Rule(p[1], p.lineno(4), args)]

    def p_simple_agg(self, p):
        'aggr : id_or_qid asKeyword id'
        args = [Field(p[1]), p[3]] # [qid, id]
        p[0] = [Rule('last', p.lineno(2), args)]
        
    def p_simple_agg_same_name(self, p):
        'aggr : id_or_qid'
        args = [Field(p[1]), p[1]] # [qid, id]
        p[0] = [Rule('last', p.lineno(1), args)]

    def p_qid(self, p):
        '''
        qid : id '.' id
        '''
        p[0] = p[1] + p[2] + p[3]
    
    def p_id_or_qid(self, p):
        '''
        id_or_qid : id
                  | qid
        '''
        p[0] = p[1]

    def p_aggr_op(self, p):
        '''
        aggr_op : minKeyword
                | maxKeyword
                | sumKeyword
                | avgKeyword
                | unionKeyword
                | countKeyword
                | bitANDKeyword
                | bitORKeyword
        '''
        p[0] = p[1]

    def p_merger(self, p):
        "merger : mergerKeyword id '{' merger_module1_n export '}'"
        p[0] = Merger(p[2], p.lineno(2), p[4], p[5])
        self.mergers.append(p[0])
        
    
    def p_merger_module1_n(self, p):
        'merger_module1_n : merger_module merger_module1_n'
        p[1].extend(p[2])
        p[0] = p[1]
    
    def p_merger_module0(self, p):
        'merger_module1_n : '
        p[0] = []
    
    def p_merger_module(self, p):
        """
        merger_module : moduleKeyword id '{' merger_branches merger_rule1_n '}'
        """
        p[0] = [Module(p[2], p.lineno(2), p[5], p[4])]
    
    def p_merger_branches(self, p):
        'merger_branches : branchesKeyword branches1_n'
        p[0] = p[2]

    def p_branches1_n(self, p):
        """
        branches1_n : id ',' branches1_n
        """
        p[0] = [p[1]]
        p[0].extend(p[3])
    
    def p_branches1(self, p):
        ' branches1_n : id'
        p[0] = [p[1]]

    def p_export(self, p):
        'export : exportKeyword id'
        p[0] = p[2]

    def p_merger_rule1_n(self, p):
        'merger_rule1_n : merger_rule merger_rule1_n'
        p[1].extend(p[2])
        p[0] = p[1]
    
    def p_merger_rule0(self,p):
        'merger_rule1_n :'
        p[0] = []

    def p_merger_rule(self, p):
        '''
        merger_rule : merger_prefix_rule
                    | merger_infix_rule
        '''
        p[0] = [[p[1]]]

    def p_not_merger_rule(self, p):
        '''
        merger_rule : NOTKeyword merger_prefix_rule
                    | NOTKeyword merger_infix_rule
        '''
        p[2].NOT = True
        p[0] = [[p[2]]]
    
    def p_merger_infix_rule(self, p):
        'merger_infix_rule : qid_arg op qid_arg'
        p[1].extend(p[3])
        p[0] = Rule(p[2][0], p[2][1], p[1])

    def p_merger_prefix_rule(self,p):
        '''
        merger_prefix_rule : id '(' qid_args ')'
        '''
        p[0] = Rule(p[1], p.lineno(1), p[3])

    def p_qid_args(self,p):
        '''
        qid_args : qid_arg ',' qid_args
        '''
        p[0] = p[1]
        p[0].extend(p[3]) # concatenate the rest of the args to arg

    def p__qid_args_more(self,p):
        'qid_args : qid_arg'
        p[0] = p[1]

    def p_no_qid_args(self, p):
        'qid_args :'
        p[0] = []

    def p_qid_arg(self, p):
        '''
        qid_arg : qid
            | IPv4
            | IPv6
            | CIDR
            | MAC
            | int
            | float
            | hex
            | merger_prefix_rule
            | string
        '''
        if type(p[1]) is type("string"):
            p[1] = Field(p[1])
        p[0] = [p[1]] # list of one element for easy [].extend later

    def p_merger_rule_al_op(self, p):
        'merger_rule : allen_rule opt_or_allen_rule'
        p[1].extend(p[2])
        p[0] = [p[1]]

    def p_opt_or_allen_rule(self, p):
        'opt_or_allen_rule : ORKeyword allen_rule opt_or_allen_rule'
        p[2].extend(p[3])
        p[0] = p[2]
    
    def p_opt_op_rule_end(self, p):
        'opt_or_allen_rule : '
        p[0] = []
    
    def p_allen_rule(self, p):
        'allen_rule : id allen_op id opt_allen_delta'
        args = [Field(p[1]), Field(p[3])]
        args.extend(p[4]) # add the delta time to [arg, arg]
        p[0] = [AllenRule(p[2], p.lineno(1), args)] # (op, line, args)
    
    def p_opt_allen_delta(self, p):
        '''
        opt_allen_delta : deltaKeyword time
        '''
        p[0] = [p[2]]
        
    def p_no_allen_delta(self, p):
        'opt_allen_delta :'
        p[0] = []

    def p_allen_op(self, p):
        '''
        allen_op : LT
                 | GT
                 | EQ
                 | mKeyword
                 | miKeyword
                 | oKeyword
                 | oiKeyword
                 | sKeyword
                 | siKeyword
                 | dKeyword
                 | diKeyword
                 | fKeyword
                 | fiKeyword
                 | eqKeyword
        '''
        # for some strange reason upper level refuses to recognize lineno:
        p[0] = p[1]

    def p_error(self, p):
        msg ="Syntax error. Unexpected token "
        msg +="%s (%s)"%(p.value, p.type)
        msg += " at line %s"% self.lexer.lineno
        raise  SyntaxError(msg)

    def parse(self, text):
        self.parser.parse(text, lexer=self.lexer)
        self.resolve_branches()
        
    def find_io_nodes(self):
        '''
        Finds which branch nodes are inputs and which are outputs.
        The rest of the branches are processing stages.
        '''
        
        pass
    
    def check_branching(self):
        pass
    
    def check_branch_nodes(self):
        for b in self.branch_nodes.values():
            if not b.is_branch:
                try:
                    node = self.names[b.name]
                    if len(b.inputs) == 0:
                        msg = "Node %s at line" % b.name
                        msg += " %s does not have input." % b.line
                        raise SyntaxError(msg)
                    if len(b.outputs) == 0:
                        msg = "Node %s at line" % b.name
                        msg += " %s does not have output." % b.line
                        raise SyntaxError(msg)
                    if len(b.inputs) > 1 and type(node) is not Merger:
                        msg = "Non-Merger node %s at line" % b.name
                        msg += " %s has more than one input." % b.line
                        raise SyntaxError(msg)
                    if len(b.outputs) > 1 and type(node) is not Splitter:
                        msg = "Non-Splitter node %s at line" % b.name
                        msg += " %s has more than one output." % b.line
                        raise SyntaxError(msg)

                except KeyError:
                    # check whether this is some middle node
                    if len(b.inputs) != 0 and len(b.outputs) !=0:
                        msg = "Node %s refferenced at line" % b.name
                        msg += " %s not defined" % b.line
                        raise SyntaxError(msg)
                    
                    #check whether the node name is actually parser string(Arg)
                    if type(b.name) is not Arg:
                        msg = "Node %s refferenced at line" % b.name
                        msg += " %s not defined" % b.line
                        raise SyntaxError(msg)
            else:
                if len(b.inputs) != 1 or len(b.outputs) != 1:
                    msg = "Branch Node %s at line" % b.name
                    msg += " %s must have 1 input and 1 output." % b.line
                    raise SyntaxError(msg)
                
                
                    
    def resolve_branches(self):
        noname_branchings = []
        for branch in self.branches:
            br_name = False
            br_index = 0
            for i, node in enumerate(branch):
                if type(node) is BranchNode:
                    try:
                        branch[i] = self.names[node.name]
                    except KeyError:
                        msg = "Node %s refferenced at line" % node.name
                        msg += " %s not defined" % node.line
                        raise SyntaxError(msg)
                if type(node) is Branch:
                    br_name = node.name
                    br_index = i
                    self.branch_names.add(br_name)
                
                if type(node) is Input and i != 0:
                    msg = "Input node %s at line" % node.name
                    msg += " %s should be at first posigion" % node.line
                    msg += " of branching statement"
                    raise SyntaxError(msg)
                
                if type(node) is Output and i != (len(branch) - 1):
                    msg = "Output node %s at line" % node.name
                    msg += " %s should be at position posigion" % node.line
                    msg += " of branching statement"
                    raise SyntaxError(msg)

            if br_name:
                del(branch[br_index])
                for node in branch:
                    node.branches.add(br_name)
            else:
                noname_branchings.append(branch)
                    
        # second iteration to fix the remaining node, which don't have branches
        for branch in noname_branchings:
            s = set()
            for node in branch:
                s.update(node.branches)
            for node in branch:
                node.branches.update(s)


class ParsedFile(object):
    def __init__(self, filters, groupers, splitters, group_filters,
                 mergers, branches, ungroupers, input, output, names):
        self.filters = filters
        self.groupers = groupers
        self.splitters = splitters
        self.group_filters = group_filters
        self.mergers = mergers
        self.branches = branches
        self.ungroupers = ungroupers
        self.input = input
        self.output = output
        self.names = names
    
    