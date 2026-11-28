# Localizar e substituir palavras
<!-- keywords: localizar avançado, soa como, soundex, formas da palavra, todas as formas de uma palavra, ignorar pontuação, ignorar espaços, especial, marca de parágrafo, tabulação, localizar por formatação, formatação, negrito, fonte, estilo, curingas, curinga, padrão, expressão regular, grupos, contagem de correspondências, realçar todas as correspondências, substituir, substituir tudo, pesquisar, procurar, alterar palavras, diferenciar maiúsculas de minúsculas, palavras inteiras, localizar próximo, ctrl+f, ctrl+h, edição, painel, texto oculto, desfazer -->

Use Localizar para ir até palavras no seu documento. Use Substituir para alterá-las. Os dois funcionam em um único painel à direita da janela.

## Localizar palavras

1. Pressione **Ctrl+F**, ou clique em **Página Inicial > Edição > Localizar**. O painel **Localizar e Substituir** é aberto, com o cursor em **Localizar**.
2. Digite as palavras que você procura. Depois pressione **Enter**, ou clique em **Localizar Próximo**.

O JeffWrite começa onde está o cursor. Ele seleciona o próximo lugar onde encontra as palavras e rola as páginas para que você possa vê-lo. Esse lugar é chamado de **correspondência**. Clique em **Localizar Próximo** de novo para ir à próxima correspondência. Depois da última, ele volta ao início do documento.

Conforme você digita, o painel conta as correspondências, como "9 correspondências", e cada uma recebe uma faixa amarela nas páginas. As faixas aparecem também em células de tabelas, cabeçalhos, rodapés e notas de rodapé. Depois de **Localizar Próximo**, o painel informa em qual você está, como "Correspondência 2 de 4". A contagem e as faixas mudam quando você clica em **Diferenciar maiúsculas de minúsculas** ou **Localizar apenas palavras inteiras**, ou quando altera o texto. As faixas continuam nas páginas quando você fecha o painel e quando clica em **Modo de Leitura**, para que você leia com todas as correspondências à vista. Nada no documento muda, e as faixas nunca são impressas nem salvas. Elas desaparecem quando você altera o documento, quando abre o painel de novo, ou quando clica em **Página Inicial > Edição > Limpar Destaques de Localização**.

## Localizar com curingas

Marque **Usar curingas** para procurar um padrão, e não apenas palavras. Um **curinga** é um sinal que representa outras letras. Estes são os sinais:

- **?** representa qualquer letra. **c?t** encontra "cat" e "cot".
- **\*** representa qualquer sequência de letras. **c\*t** encontra "cat" e "cart".
- **<** é o início de uma palavra, e **>** é o fim. **<the>** encontra "the", mas não "other".
- **[abc]** é qualquer uma dessas letras, **[a-c]** é qualquer letra de a a c, e **[!abc]** é qualquer letra, exceto essas.
- **{2}** repete duas vezes a letra anterior. **{1,3}** repete de uma a três vezes.
- **@** significa uma ou mais vezes a letra anterior.
- **\\** antes de um sinal encontra o próprio sinal. **\\?** encontra um ponto de interrogação.
- **^p** é uma marca de parágrafo, **^t** é uma tabulação, **^#** é um algarismo, **^$** é uma letra, e **^w** é um espaço em branco.

Coloque parte do padrão entre **( )** para criar um **grupo**. Em **Substituir por**, **\1** é o primeiro grupo, **\2** é o segundo, e assim por diante até **\9**. **^&** é tudo o que foi encontrado. Por exemplo, procure **(<[A-Z]@>), (<[A-Z]@>)** e substitua por **\2 \1** para transformar "Silva, João" em "João Silva". **Localizar apenas palavras inteiras** fica desativado enquanto os curingas estiverem ligados, porque **<** e **>** fazem esse papel.

Se o padrão tiver algum erro, o painel informa qual é.

Se as suas palavras não estiverem no documento, o painel avisa, e nada se move.

Você pode continuar trabalhando no documento enquanto o painel está aberto. Clique na página para digitar. Clique no **X** no alto do painel para fechá-lo.

## Localizar com expressões regulares

Marque **Usar expressões regulares** para padrões mais avançados que os curingas. Uma **expressão regular** é um padrão curto formado por letras e sinais. Ela é ativada no lugar de **Usar curingas**: apenas uma das duas pode estar ligada. Estes são os sinais mais comuns:

- **.** é qualquer caractere, **\d** é um algarismo, **\w** é uma letra, algarismo ou sublinhado, e **\s** é um espaço ou tabulação. **[a-c]** é qualquer letra de a a c, e **[^abc]** é qualquer caractere exceto esses.
- **^** é o início de um parágrafo, **$** é o fim de um parágrafo, e **\b** é a borda de uma palavra. **\bgato\b** encontra "gato", mas não "gatos".
- **\*** significa nenhuma ou mais vezes o que vem antes, **+** significa uma ou mais vezes, e **?** significa nenhuma ou uma vez. **{2}** significa exatamente duas vezes, e **{2,5}** significa de duas a cinco vezes; a contagem pode ir até 200. Acrescente **?** depois de uma repetição, como em **.+?**, para encontrar o trecho mais curto possível.
- **|** significa "ou": **gato|cão** encontra qualquer uma das duas palavras.
- **( )** cria um **grupo**. Use **(?: )** para um grupo que você não precisa em **Substituir por**.
- **\\** antes de um sinal encontra o próprio sinal: **\.** encontra um ponto. **\t** é uma tabulação.

**Diferenciar maiúsculas de minúsculas** funciona como para palavras. Letras com acento e letras de outros alfabetos contam como letras.

Um padrão é pesquisado um parágrafo de cada vez, então **^** e **$** significam o início e o fim de cada parágrafo. Para pesquisar atravessando o fim de um parágrafo, coloque **\n** (uma marca de parágrafo) no padrão: **lar\.\nSilva** encontra "lar." no fim de um parágrafo e "Silva" no início do seguinte.

Em **Substituir por**, **$1** ou **\1** é o primeiro grupo, **$2** ou **\2** é o segundo, e assim por diante até nove grupos. **$0** ou **\0** é tudo o que foi encontrado. **$$** é um cifrão, **\\** é uma barra invertida, e **\t** é uma tabulação. Por exemplo, procure **(\w+), (\w+)** e substitua por **$2 $1** para transformar "Silva, João" em "João Silva".

**Substituir Tudo** altera todas as correspondências no texto, cabeçalhos, rodapés, notas e caixas de texto em uma única ação, então um único **Desfazer** desfaz todas elas. Com **Controlar Alterações** ligado, cada mudança é controlada, como acontece com as palavras. O texto oculto não é alterado.

Se o padrão tiver algum erro, o painel informa perto de qual caractere. Um padrão pode não casar com nenhum caractere, como **^** (o início de um parágrafo) ou **$** (o fim): **Substituir por** então coloca as suas palavras nesse ponto, de modo que **^** com **> ** adiciona esses sinais no início de cada parágrafo. Um padrão que levaria tempo demais no seu texto, como **(a+)+$** em uma linha longa de a's, é interrompido pelo JeffWrite, que informa isso; nada é alterado. Uma pesquisa demorada mostra uma caixa **Pesquisando** com um botão **Cancelar**; cancelar deixa o documento como estava, e você não pode digitar no documento até a pesquisa terminar. Padrões que olham à frente ou atrás sem consumir o que verificam, como **x(?=\d)**, e padrões no modo estendido, como **(?x)**, não são aceitos; o JeffWrite informa isso. Um parágrafo muito longo é pesquisado em partes, e cada correspondência é a mesma que uma pesquisa do parágrafo inteiro encontraria. Um padrão que case com um trecho muito longo de um parágrafo muito longo (dezenas de milhares de caracteres) pode ser interrompido com a mensagem "precisou de trabalho demais", e uma única correspondência com mais de 200.000 caracteres não é substituída: o JeffWrite para, informa isso e não altera nada.

## Localizar avançado

Estas opções estão no painel, abaixo de **Usar expressões regulares**. Cada uma muda a forma como o JeffWrite lê as palavras em **Localizar**.

- **Soa como** encontra palavras que são pronunciadas do mesmo jeito que a sua palavra. Ele dá a cada palavra um código a partir dos seus sons e encontra as palavras com o mesmo código. Digite **Silva**, e ele também encontra "Silvia".
- **Localizar todas as formas da palavra** encontra todas as formas da palavra. Digite **correr**, e ele encontra "corre," "correndo" e "corremos". Funciona com palavras inteiras. Se você digitar duas palavras, como **cão grande**, ele as encontra lado a lado, em qualquer uma de suas formas.
- **Ignorar caracteres de pontuação** pula sinais como hífens e pontos entre as letras. Digite **email**, e ele também encontra "e-mail".
- **Ignorar caracteres de espaço em branco** pula espaços e tabulações entre as letras. Digite **e mail**, e ele encontra "email".

Apenas uma entre **Soa como**, **Localizar todas as formas da palavra**, **Usar curingas** e **Usar expressões regulares** pode estar ligada de cada vez. **Soa como** e **Localizar todas as formas da palavra** sempre encontram palavras inteiras, então **Localizar apenas palavras inteiras** e **Diferenciar maiúsculas de minúsculas** ficam desativados.

### Especial

Clique em **Especial** para colocar um código em **Localizar**: **Marca de Parágrafo** (^p), **Caractere de Tabulação** (^t), **Qualquer Dígito** (^#), **Qualquer Letra** (^$), **Espaço em Branco** (^w), **Quebra de Seção** (^b), **Campo** (^d), **Marca de Nota de Rodapé** (^f), **Marca de Nota de Fim** (^e) ou **Caractere de Circunflexo** (^^). Os códigos funcionam sem **Usar curingas**. Uma **quebra de seção** é a marca que encerra uma seção. Um **campo** é um trecho de texto que o programa calcula, como um número de página ou uma data, e **^d** encontra onde cada um começa. As marcas de nota são os pequenos números no texto que apontam para uma nota de rodapé ou uma nota de fim. Esses quatro são encontrados, mas **Substituir** e **Substituir Tudo** não os alteram, porque mudar um deles faria perder aquilo que ele representa. Os outros sinais da caixa, como **?** e **\***, passam a ser apenas esses caracteres.

### Formatação

Clique em **Formatar** para encontrar apenas o texto com determinada aparência: **Negrito**, **Itálico**, **Sublinhado**, **Tachado**, **Realçado**, uma **Fonte** ou um **Estilo** de parágrafo. O painel lista as aparências que você escolheu sob os botões. Escolha **Sem Formatação** para limpá-las.

Você pode deixar **Localizar** vazio. Então, cada trecho de texto com essa aparência é uma correspondência. Com palavras em **Localizar**, somente as palavras com essa aparência são encontradas. **Substituir Tudo** altera apenas essas palavras.

## Restringir a pesquisa

- **Diferenciar maiúsculas de minúsculas** encontra apenas palavras com as mesmas letras maiúsculas. Com essa opção ligada, "Plano" não encontra "plano".
- **Localizar apenas palavras inteiras** pula palavras maiores que contêm a sua. Com essa opção ligada, "plano" não encontra "planeta".
- **Pesquisar** define em que sentido **Localizar Próximo** segue a partir do cursor. **Para Baixo** vai até o fim do documento, e **Para Cima** vai até o início. Em cada caso, ele para ali e informa isso. **Tudo** continua até a outra extremidade e é a escolha usual. **Substituir Tudo** sempre altera o documento inteiro.

## Substituir palavras

1. Pressione **Ctrl+H**, ou clique em **Página Inicial > Edição > Substituir**. O cursor vai para **Substituir por**.
2. Digite as palavras novas em **Substituir por**. Para remover as palavras encontradas, deixe o campo vazio.
3. Digite as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira correspondência. Clique em **Substituir** de novo para alterá-la. Depois o JeffWrite passa para a próxima correspondência. Cada alteração é uma etapa, então **Desfazer** (Ctrl+Z) desfaz uma de cada vez.
5. Clique em **Substituir Tudo** para alterar todas as correspondências de uma vez. O painel informa quantas foram alteradas. **Substituir Tudo** também altera as palavras em caixas de texto e formas, e é uma única ação, então um único **Desfazer** devolve todas as palavras. Quando **Controlar Alterações** está ligado, cada substituição é registrada como alteração controlada, no texto e nas caixas. Uma caixa grande demais para ser alterada é contada na mensagem. **Substituir Tudo** não alcança caixas de texto em cabeçalhos e rodapés, formas agrupadas nem caixas vinculadas entre si. **Localizar Próximo** ainda não procura em caixas de texto.

As novas palavras seguem as maiúsculas e minúsculas da primeira letra das palavras que substituem. Se você substituir uma palavra em negrito, a nova palavra fica em negrito.

Por exemplo, para trocar "cor" por "cores" em todo o documento: digite **cor** em **Localizar**, digite **cores** em **Substituir por** e clique em **Substituir Tudo**.

## O que a pesquisa examina

- Ela examina o texto do documento, inclusive o texto das tabelas. Um indicador no meio de algumas palavras, que alguns arquivos têm, não impede que elas sejam encontradas. **Substituir** e **Substituir Tudo** mantêm o indicador.
- **Localizar Próximo** examina primeiro o texto. Depois passa aos cabeçalhos, em seguida aos rodapés, às notas de rodapé e, por fim, volta ao início do texto. Quando uma correspondência está em um desses lugares, o JeffWrite o abre para edição e seleciona a correspondência. O painel informa onde ela está, como "Encontrado no cabeçalho da página 3." **Substituir** troca a correspondência em que você está, no texto ou em um cabeçalho, rodapé ou nota de rodapé. **Substituir Tudo** altera todas as correspondências em todos eles, e um único **Desfazer** traz todas de volta. As notas de fim fazem parte do texto, então são pesquisadas com ele. Veja [Cabeçalhos e rodapés](headers-footers).
- Ela ignora o texto definido como **Oculto**, porque esse texto não é mostrado. Veja [Alterar a aparência do texto](font).

## Veja também

- [Alterar a aparência do texto](font)
- [Usar o JeffWrite com o teclado](keyboard)
