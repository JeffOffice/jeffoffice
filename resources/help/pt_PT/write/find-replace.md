# Localizar e substituir palavras
<!-- keywords: localizar avançado, soa como, soundex, formas da palavra, todas as formas de uma palavra, ignorar pontuação, ignorar espaços, especial, marca de parágrafo, tabulação, localizar por formatação, formatação, negrito, tipo de letra, estilo, caracteres universais, caráter universal, padrão, expressão regular, grupos, contagem de correspondências, realçar todas as correspondências, substituir, substituir tudo, pesquisar, procurar, alterar palavras, diferenciar maiúsculas, palavras inteiras, localizar seguinte, ctrl+f, ctrl+h, edição, painel, texto oculto, anular -->

Use **Localizar** para ir até palavras no documento. Use **Substituir** para as alterar. Ambas funcionam num único painel do lado direito da janela.

## Localizar palavras

1. Prima **Ctrl+F**, ou clique em **Base > Edição > Localizar**. Abre-se o painel **Localizar e Substituir**, com o cursor em **Localizar**.
2. Escreva as palavras que pretende. Depois prima **Enter**, ou clique em **Localizar seguinte**.

O JeffWrite começa onde estiver o cursor. Seleciona o próximo lugar onde encontra as suas palavras e desloca as páginas para que o possa ver. Esse lugar chama-se **correspondência**. Clique em **Localizar seguinte** de novo para ir à correspondência seguinte. Depois da última correspondência, volta ao início do documento.

À medida que escreve, o painel conta as correspondências, como "9 correspondências", e cada uma recebe uma faixa amarela nas páginas. As faixas aparecem também em células de tabelas, cabeçalhos, rodapés e notas de rodapé. Depois de **Localizar seguinte**, o painel indica em qual está, como "Correspondência 2 de 4". A contagem e as faixas mudam quando clica em **Diferenciar maiúsculas** ou **Localizar apenas palavras inteiras**, ou quando altera o texto. As faixas permanecem nas páginas quando fecha o painel, e quando clica em **Modo de Leitura**, para que possa ler com todas as correspondências visíveis. Nada no documento muda, e as faixas nunca são impressas nem guardadas. Desaparecem quando altera o documento, quando volta a abrir o painel, ou quando clica em **Base > Edição > Limpar Realces de Localização**.

## Localizar com caracteres universais

Marque **Utilizar caracteres universais** para pesquisar um padrão, e não apenas palavras. Um **caráter universal** é uma marca que representa outras letras. Estas são as marcas:

- **?** representa uma letra qualquer. **c?t** encontra "cat" e "cot".
- **\*** representa qualquer sequência de letras. **c\*t** encontra "cat" e "cart".
- **<** é o início de uma palavra, e **>** é o fim. **<the>** encontra "the" mas não "other".
- **[abc]** é uma destas letras, **[a-c]** é qualquer letra de a a c, e **[!abc]** é qualquer letra exceto estas.
- **{2}** repete duas vezes a letra que a antecede. **{1,3}** repete-a de uma a três vezes.
- **@** significa uma ou mais vezes a letra que a antecede.
- **\\** antes de uma marca encontra a própria marca. **\\?** encontra um ponto de interrogação.
- **^p** é uma marca de parágrafo, **^t** é uma tabulação, **^#** é um algarismo, **^$** é uma letra, e **^w** é um espaço em branco.

Coloque parte do padrão entre **( )** para criar um **grupo**. Em **Substituir por**, **\1** é o primeiro grupo, **\2** é o segundo, e assim sucessivamente até **\9**. **^&** é tudo o que foi encontrado. Por exemplo, procure **(<[A-Z]@>), (<[A-Z]@>)** e substitua por **\2 \1** para transformar "Silva, João" em "João Silva". **Localizar apenas palavras inteiras** fica esbatido enquanto os caracteres universais estiverem ativos, porque **<** e **>** fazem esse trabalho.

Se o padrão tiver um erro, o painel indica qual.

Se as suas palavras não estiverem no documento, o painel informa e nada se move.

Pode continuar a trabalhar no documento enquanto o painel estiver aberto. Clique na página para escrever. Clique no **X** no topo do painel para o fechar.

## Localizar com expressões regulares

Marque **Utilizar expressões regulares** para padrões mais elaborados do que os caracteres universais. Uma **expressão regular** é um padrão curto, feito de letras e marcas. Ativa-se em vez de **Utilizar caracteres universais**: só um dos dois pode estar ativo. Estas são as marcas mais comuns:

- **.** é qualquer carácter, **\d** é um algarismo, **\w** é uma letra, algarismo ou sublinhado, e **\s** é um espaço ou uma tabulação. **[a-c]** é qualquer letra de a a c, e **[^abc]** é qualquer carácter exceto estes.
- **^** é o início de um parágrafo, **$** é o fim, e **\b** é a fronteira de uma palavra. **\bgato\b** encontra "gato" mas não "gatos".
- **\*** significa nenhuma ou mais vezes o que vem antes, **+** significa uma ou mais vezes, e **?** significa nenhuma ou uma vez. **{2}** significa exatamente duas vezes, e **{2,5}** significa de duas a cinco vezes; a contagem pode ir até 200. Acrescente **?** depois de uma repetição, como em **.+?**, para encontrar a sequência mais curta que se ajusta.
- **|** significa "ou": **gato|cão** encontra qualquer uma das palavras.
- **( )** cria um **grupo**. Use **(?: )** para um grupo de que não precisa em **Substituir por**.
- **\\** antes de uma marca encontra a própria marca: **\.** encontra um ponto final. **\t** é uma tabulação.

**Diferenciar maiúsculas** funciona como para as palavras. As letras com acentos e as letras de outros alfabetos contam como letras.

Um padrão é pesquisado um parágrafo de cada vez, por isso **^** e **$** significam o início e o fim de cada parágrafo. Para pesquisar através do fim de um parágrafo, coloque **\n** (uma marca de parágrafo) no padrão: **casa\.\nSilva** encontra "casa." no fim de um parágrafo e "Silva" no início do seguinte.

Em **Substituir por**, **$1** ou **\1** é o primeiro grupo, **$2** ou **\2** é o segundo, e assim sucessivamente até nove grupos. **$0** ou **\0** é tudo o que foi encontrado. **$$** é um símbolo de dólar, **\\** é uma barra invertida, e **\t** é uma tabulação. Por exemplo, procure **(\w+), (\w+)** e substitua por **$2 $1** para transformar "Silva, João" em "João Silva".

**Substituir tudo** altera cada correspondência no texto, nos cabeçalhos, nos rodapés, nas notas e nas caixas de texto, num único passo, por isso um só **Anular** repõe todas. Com **Controlar Alterações** ativo, cada alteração é registada, tal como acontece com as palavras. O texto oculto não é alterado.

Se o padrão tiver um erro, o painel indica junto a que carácter. Um padrão pode não corresponder a nenhum carácter, como **^** (o início de um parágrafo) ou **$** (o fim): **Substituir por** insere então o seu texto nesse lugar, por isso **^** com **> ** acrescenta essas marcas no início de cada parágrafo. Um padrão que demoraria tempo excessivo no seu texto, como **(a+)+$** numa linha longa de a, é interrompido pelo JeffWrite, que o indica; nada é alterado. Uma pesquisa longa mostra uma caixa **A pesquisar** com um botão **Cancelar**; cancelar deixa o documento como estava, e não pode escrever no documento até a pesquisa terminar. Padrões que olham para a frente ou para trás sem consumir o que verificam, como **x(?=\d)**, e padrões em modo estendido, como **(?x)**, não são suportados; o JeffWrite indica-o. Um parágrafo muito longo é pesquisado por partes, e cada correspondência é a que uma pesquisa do parágrafo inteiro encontraria. Um padrão que corresponda a um trecho muito longo de um parágrafo muito extenso (dezenas de milhares de caracteres) pode ser interrompido com a mensagem de "trabalho excessivo", e uma única correspondência com mais de 200 000 caracteres não é substituída: o JeffWrite para, indica-o e não altera nada.

## Localizar avançado

Estas opções estão no painel, em **Utilizar expressões regulares**. Cada uma altera a forma como o JeffWrite lê as palavras em **Localizar**.

- **Soa como** encontra palavras que se dizem da mesma forma que a sua palavra. Atribui a cada palavra um código a partir dos seus sons, e encontra as palavras com o mesmo código. Escreva **Silva**, e encontra também "Sylva".
- **Localizar todas as formas da palavra** encontra todas as formas da palavra. Escreva **correr**, e encontra "corre", "correu" e "correndo". Funciona com palavras inteiras. Se escrever duas palavras, como **cão grande**, encontra-as lado a lado, em qualquer das suas formas.
- **Ignorar carateres de pontuação** salta marcas como hífenes e pontos entre as letras. Escreva **email**, e encontra também "e-mail".
- **Ignorar carateres de espaço em branco** salta espaços e tabulações entre as letras. Escreva **e mail**, e encontra "email".

Só uma das opções **Soa como**, **Localizar todas as formas da palavra**, **Utilizar caracteres universais** e **Utilizar expressões regulares** pode estar ativa de cada vez. **Soa como** e **Localizar todas as formas da palavra** encontram sempre palavras inteiras, por isso **Localizar apenas palavras inteiras** e **Diferenciar maiúsculas** ficam esbatidos.

### Especial

Clique em **Especial** para colocar um código em **Localizar**: **Marca de Parágrafo** (^p), **Carácter de Tabulação** (^t), **Qualquer Algarismo** (^#), **Qualquer Letra** (^$), **Espaço em Branco** (^w), **Quebra de Secção** (^b), **Campo** (^d), **Marca de Nota de Rodapé** (^f), **Marca de Nota de Fim** (^e), ou **Carácter de Circunflexo** (^^). Os códigos funcionam sem **Utilizar caracteres universais**. Uma **quebra de secção** é a marca que termina uma secção. Um **campo** é um pedaço de texto que o programa calcula, como um número de página ou uma data, e **^d** encontra onde cada um começa. As marcas de nota são os pequenos números no texto que apontam para uma nota de rodapé ou de fim. Estas quatro são encontradas, mas **Substituir** e **Substituir tudo** não lhes tocam, porque alterar uma delas faria perder aquilo que representa. Os restantes símbolos da caixa, como **?** e **\***, são então apenas esses carateres.

### Formatação

Clique em **Formatação** para encontrar apenas texto com um certo aspeto: **Negrito**, **Itálico**, **Sublinhado**, **Rasurado**, **Realçado**, um **Tipo de letra**, ou um **Estilo** de parágrafo. O painel lista os aspetos que escolheu por baixo dos botões. Escolha **Sem Formatação** para os limpar.

Pode deixar **Localizar** vazio. Então, cada trecho de texto com esse aspeto é uma correspondência. Com palavras em **Localizar**, só são encontradas as palavras com esse aspeto. **Substituir tudo** altera apenas essas palavras.

## Restringir a pesquisa

- **Diferenciar maiúsculas** encontra apenas palavras com as mesmas maiúsculas. Com esta opção ativa, "Plano" não encontra "plano".
- **Localizar apenas palavras inteiras** salta palavras mais longas que contenham a sua. Com esta opção ativa, "plano" não encontra "planeta".
- **Pesquisar** define em que sentido **Localizar seguinte** avança a partir do cursor. **Para baixo** vai até ao fim do documento, e **Para cima** vai até ao início. Cada um pára aí e indica-o. **Tudo** continua até à outra extremidade e é a escolha habitual. **Substituir tudo** altera sempre o documento inteiro.

## Substituir palavras

1. Prima **Ctrl+H**, ou clique em **Base > Edição > Substituir**. O cursor vai para **Substituir por**.
2. Escreva as palavras novas em **Substituir por**. Para remover as palavras encontradas, deixe o campo vazio.
3. Escreva as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira correspondência. Clique em **Substituir** de novo para a alterar. O JeffWrite passa então à correspondência seguinte. Cada alteração é um passo, por isso **Anular** (Ctrl+Z) retira uma de cada vez.
5. Clique em **Substituir tudo** para alterar todas as correspondências de uma vez. O painel indica quantas alterou. **Substituir tudo** altera também as palavras em caixas de texto e formas, e é um único passo, por isso um só **Anular** repõe todas as palavras. Quando **Controlar Alterações** está ativo, cada substituição fica registada como alteração controlada, no texto e nas caixas. Uma caixa demasiado grande para ser alterada é contada na mensagem. **Substituir tudo** não chega às caixas de texto em cabeçalhos e rodapés, às formas agrupadas, nem às caixas ligadas entre si. **Localizar seguinte** ainda não procura nas caixas de texto.

As palavras novas assumem o aspeto da primeira letra das palavras que substituem. Se substituir uma palavra a negrito, a palavra nova fica a negrito.

Por exemplo, para alterar "cor" para "côr" em todo o documento: escreva **cor** em **Localizar**, escreva **côr** em **Substituir por**, e clique em **Substituir tudo**.

## O que a pesquisa analisa

- Analisa o texto do documento, incluindo o texto nas tabelas. Um marcador no meio de algumas palavras, que alguns ficheiros têm, não impede que essas palavras sejam encontradas. **Substituir** e **Substituir tudo** mantêm o marcador.
- **Localizar seguinte** procura primeiro no texto. Depois passa aos cabeçalhos, depois aos rodapés, depois às notas de rodapé, e por fim volta ao início do texto. Quando uma correspondência está num deles, o JeffWrite abre-o para edição e seleciona a correspondência. O painel indica onde está, como "Encontrado no cabeçalho da página 3." **Substituir** troca a correspondência em que está, no texto ou num cabeçalho, rodapé ou nota de rodapé. **Substituir tudo** altera cada correspondência em todos eles, e um só **Anular** as repõe todas. As notas de fim fazem parte do texto, por isso são pesquisadas com ele. Veja [Cabeçalhos e rodapés](headers-footers).
- Ignora o texto definido como **Oculto**, porque esse texto não é apresentado. Veja [Alterar o aspeto do texto](font).

## Consulte também

- [Alterar o aspeto do texto](font)
- [Utilizar o JeffWrite com o teclado](keyboard)
