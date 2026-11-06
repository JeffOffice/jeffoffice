# Localizar e substituir palavras

<!-- keywords: localização avançada, soa como, soundex, formas de palavra, todas as formas de uma palavra, ignorar pontuação, ignorar espaços, especial, marca de parágrafo, tabulação, localizar por formato, formato, negrito, tipo de letra, estilo, caracteres universais, caráter universal, padrão, expressão regular, grupos, contagem de correspondências, realçar todas as correspondências, substituir, substituir tudo, pesquisar, procurar, alterar palavras, maiúsculas e minúsculas, palavras inteiras, localizar seguinte, ctrl+f, ctrl+h, edição, painel, texto oculto, anular -->

Use **Localizar** para ir até palavras no seu documento. Use **Substituir** para as alterar. Ambas as funções funcionam num único painel à direita da janela.

## Localizar palavras

1. Prima **Ctrl+F** ou clique em **Base > Edição > Localizar**. O painel **Localizar e Substituir** abre-se, com o cursor em **Localizar**.
2. Escreva as palavras que pretende. Depois prima **Enter** ou clique em **Localizar seguinte**.

O JeffWrite começa onde estiver o cursor. Seleciona o próximo lugar onde encontra as suas palavras e desloca as páginas para que o possa ver. Esse lugar chama-se **correspondência**. Clique em **Localizar seguinte** novamente para ir para a correspondência seguinte. Depois da última correspondência, volta ao início do documento.

À medida que escreve, o painel conta as correspondências, por exemplo "9 correspondências", e cada uma recebe uma faixa amarela nas páginas. As faixas aparecem também em células de tabelas, cabeçalhos, rodapés e notas de rodapé. Depois de clicar em **Localizar seguinte**, o painel indica em qual está, por exemplo "Correspondência 2 de 4". A contagem e as faixas mudam quando clica em **Maiúsculas e minúsculas** ou em **Localizar apenas palavras inteiras**, ou quando altera o texto. Desaparecem quando fecha o painel.

## Localizar com caracteres universais

Marque **Utilizar caracteres universais** para pesquisar um padrão, e não apenas palavras. Um **caráter universal** é um sinal que representa outras letras. Estes são os sinais:

- **?** representa uma letra qualquer. **c?t** encontra "cat" e "cot".
- **\*** representa uma sequência qualquer de letras. **c\*t** encontra "cat" e "cart".
- **<** é o início de uma palavra, e **>** é o fim. **<the>** encontra "the", mas não "other".
- **[abc]** é uma dessas letras, **[a-c]** é qualquer letra de a a c, e **[!abc]** é qualquer letra exceto essas.
- **{2}** repete duas vezes a letra anterior. **{1,3}** repete-a de uma a três vezes.
- **@** significa uma ou mais vezes a letra anterior.
- **\\** antes de um sinal encontra o próprio sinal. **\\?** encontra um ponto de interrogação.
- **^p** é uma marca de parágrafo, **^t** é uma tabulação, **^#** é um algarismo, **^$** é uma letra, e **^w** é um espaço em branco.

Coloque parte do padrão entre **( )** para criar um **grupo**. Em **Substituir por**, **\1** é o primeiro grupo, **\2** é o segundo, e assim sucessivamente até **\9**. **^&** é tudo o que foi encontrado. Por exemplo, procure **(<[A-Z]@>), (<[A-Z]@>)** e substitua por **\2 \1** para transformar "Silva, João" em "João Silva". **Localizar apenas palavras inteiras** fica esbatido enquanto os caracteres universais estiverem ativos, porque **<** e **>** fazem esse trabalho.

Se o padrão tiver um erro, o painel indica qual é.

Se as suas palavras não estiverem no documento, o painel avisa e nada se move.

Pode continuar a trabalhar no documento enquanto o painel está aberto. Clique na página para escrever. Clique no **X** no topo do painel para o fechar.

## Localização avançada

Estas opções estão no painel, abaixo de **Utilizar caracteres universais**. Cada uma altera a forma como o JeffWrite lê as palavras em **Localizar**.

- **Soa como** encontra palavras que se pronunciam da mesma forma que a sua palavra. Atribui a cada palavra um código a partir dos seus sons e encontra as palavras com o mesmo código. Escreva **Silva**, e também encontra "Silveira".
- **Localizar todas as formas da palavra** encontra todas as formas da palavra. Escreva **correr**, e encontra "corre", "correram" e "correndo". Funciona com palavras inteiras. Se escrever duas palavras, como **cão grande**, encontra-as lado a lado, em qualquer das suas formas.
- **Ignorar caracteres de pontuação** ignora sinais como hífenes e pontos entre as letras. Escreva **e-mail**, e também encontra "email".
- **Ignorar caracteres de espaço em branco** ignora espaços e tabulações entre as letras. Escreva **e mail**, e encontra "email".

Só uma das opções **Soa como**, **Localizar todas as formas da palavra** e **Utilizar caracteres universais** pode estar ativa de cada vez. **Soa como** e **Localizar todas as formas da palavra** encontram sempre palavras inteiras, por isso **Localizar apenas palavras inteiras** e **Maiúsculas e minúsculas** ficam esbatidas.

### Especial

Clique em **Especial** para colocar um código em **Localizar**: **Marca de Parágrafo** (^p), **Carácter de Tabulação** (^t), **Qualquer Algarismo** (^#), **Qualquer Letra** (^$), **Espaço em Branco** (^w), **Quebra de Secção** (^b), **Campo** (^d), **Marca de Nota de Rodapé** (^f), **Marca de Nota de Fim** (^e) ou **Carácter de Circunflexo** (^^). Os códigos funcionam sem **Utilizar caracteres universais**. Uma **quebra de secção** é a marca que termina uma secção. Um **campo** é um pedaço de texto que o programa calcula, como um número de página ou uma data, e **^d** encontra onde cada um começa. As marcas de notas são os pequenos números no texto que remetem para uma nota de rodapé ou nota de fim. Estes quatro são encontrados, mas **Substituir** e **Substituir tudo** não lhes mexem, porque alterar um deles faria perder aquilo que representa. Os outros sinais da caixa, como **?** e **\***, passam a ser apenas esses carateres.

### Formato

Clique em **Formato** para localizar apenas texto com um determinado aspeto: **Negrito**, **Itálico**, **Sublinhado**, **Rasurado**, **Realçado**, um **Tipo de letra** ou um **Estilo** de parágrafo. O painel lista os aspetos escolhidos abaixo dos botões. Escolha **Sem formatação** para os limpar.

Pode deixar **Localizar** vazio. Então, cada trecho de texto com esse aspeto é uma correspondência. Com palavras em **Localizar**, só são encontradas as palavras com esse aspeto. **Substituir tudo** altera apenas essas palavras.

## Restringir a pesquisa

- **Maiúsculas e minúsculas** encontra apenas palavras com as mesmas maiúsculas. Com esta opção ativa, "Plano" não encontra "plano".
- **Localizar apenas palavras inteiras** ignora palavras mais longas que contenham a sua. Com esta opção ativa, "plano" não encontra "planeta".

## Substituir palavras

1. Prima **Ctrl+H** ou clique em **Base > Edição > Substituir**. O cursor passa para **Substituir por**.
2. Escreva as palavras novas em **Substituir por**. Para remover as palavras encontradas, deixe o campo vazio.
3. Escreva as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira correspondência. Clique em **Substituir** novamente para a alterar. O JeffWrite passa então para a correspondência seguinte. Cada alteração é um passo, por isso **Anular** (Ctrl+Z) desfaz uma de cada vez.
5. Clique em **Substituir tudo** para alterar todas as correspondências de uma só vez. O painel indica quantas foram alteradas. **Substituir tudo** é um único passo, por isso um **Anular** repõe todas as palavras.

As palavras novas assumem o aspeto da primeira letra das palavras que substituem. Se substituir uma palavra em negrito, a palavra nova fica em negrito.

Por exemplo, para trocar "cor" por "côr" em todo o documento: escreva **cor** em **Localizar**, escreva **côr** em **Substituir por** e clique em **Substituir tudo**.

## O que a pesquisa analisa

- Analisa o texto do documento, incluindo o texto das tabelas. Um marcador no meio de algumas palavras, que alguns ficheiros têm, não impede que sejam encontradas. **Substituir** e **Substituir tudo** mantêm o marcador.
- **Localizar seguinte** procura primeiro no texto. Depois passa aos cabeçalhos, depois aos rodapés, depois às notas de rodapé e, por fim, volta ao início do texto. Quando uma correspondência está num destes elementos, o JeffWrite abre-o para edição e seleciona a correspondência. O painel indica onde está, por exemplo "Encontrado no cabeçalho da página 3." **Substituir** troca a correspondência em que está, no texto ou num cabeçalho, rodapé ou nota de rodapé. **Substituir tudo** altera todas as correspondências em todos eles, e um **Anular** traz todas de volta. As notas de fim fazem parte do texto, por isso são pesquisadas com ele. Consulte [Cabeçalhos e rodapés](headers-footers).
- Ignora o texto definido como **Oculto**, porque esse texto não é apresentado. Consulte [Alterar o aspeto do texto](font).

## Relacionados

- [Alterar o aspeto do texto](font)
- [Utilizar o JeffWrite com o teclado](keyboard)
