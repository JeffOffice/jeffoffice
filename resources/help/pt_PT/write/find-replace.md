# Localizar e substituir palavras
<!-- keywords: localizar avançado, soa como, soundex, formas da palavra, todas as formas de uma palavra, ignorar pontuação, ignorar espaços, especial, marca de parágrafo, tabulação, localizar por formatação, formatação, negrito, tipo de letra, estilo, caracteres universais, caractere universal, padrão, expressão regular, grupos, contagem de correspondências, realçar todas as correspondências, substituir, substituir tudo, pesquisar, procurar, alterar palavras, corresponder maiúsculas, palavras inteiras, localizar seguinte, ctrl+f, ctrl+h, edição, painel, texto oculto, anular -->

Use **Localizar** para ir até palavras no seu documento. Use **Substituir** para as alterar. Ambas as funções funcionam num único painel à direita da janela.

## Localizar palavras

1. Prima **Ctrl+F**, ou clique em **Base > Edição > Localizar**. O painel **Localizar e Substituir** abre-se, com o cursor em **Localizar**.
2. Escreva as palavras que pretende. Depois prima **Enter**, ou clique em **Localizar seguinte**.

O JeffWrite começa onde estiver o cursor. Seleciona o próximo lugar onde encontra as suas palavras e desloca as páginas para que o veja. Esse lugar chama-se **correspondência**. Clique em **Localizar seguinte** novamente para ir para a correspondência seguinte. Depois da última correspondência, volta ao início do documento.

À medida que escreve, o painel conta as correspondências, por exemplo "9 correspondências", e cada uma recebe uma faixa amarela nas páginas. As faixas aparecem também em células de tabelas, cabeçalhos, rodapés e notas de rodapé. Depois de **Localizar seguinte**, o painel indica em qual está, por exemplo "Correspondência 2 de 4". A contagem e as faixas mudam quando clica em **Corresponder maiúsculas e minúsculas** ou em **Localizar apenas palavras inteiras**, ou quando altera o texto. Desaparecem quando fecha o painel.

## Localizar com caracteres universais

Marque **Utilizar caracteres universais** para procurar um padrão, e não apenas palavras. Um **caractere universal** é um sinal que representa outras letras. Estes são os sinais:

- **?** representa qualquer uma letra. **c?t** encontra "cat" e "cot".
- **\*** representa qualquer sequência de letras. **c\*t** encontra "cat" e "cart".
- **<** é o início de uma palavra, e **>** é o fim de uma palavra. **<the>** encontra "the", mas não "other".
- **[abc]** é qualquer uma destas letras, **[a-c]** é qualquer letra de a a c, e **[!abc]** é qualquer letra exceto estas.
- **{2}** repete duas vezes a letra que está antes. **{1,3}** repete de uma a três vezes.
- **@** significa uma ou mais vezes a letra que está antes.
- **\\** antes de um sinal encontra o próprio sinal. **\\?** encontra um ponto de interrogação.
- **^p** é uma marca de parágrafo, **^t** é uma tabulação, **^#** é um algarismo, **^$** é uma letra, e **^w** é um espaço em branco.

Coloque parte do padrão entre **( )** para criar um **grupo**. Em **Substituir por**, **\1** é o primeiro grupo, **\2** é o segundo, e assim por diante até **\9**. **^&** é tudo o que foi encontrado. Por exemplo, procure **(<[A-Z]@>), (<[A-Z]@>)** e substitua por **\2 \1** para transformar "Smith, John" em "John Smith". **Localizar apenas palavras inteiras** fica a cinzento enquanto os caracteres universais estiverem ativos, porque **<** e **>** fazem esse trabalho.

Se o padrão tiver um erro, o painel indica qual é.

Se as suas palavras não estiverem no documento, o painel informa-o, e nada se move.

Pode continuar a trabalhar no documento enquanto o painel estiver aberto. Clique na página para escrever. Clique no **X** no topo do painel para o fechar.

## Localizar avançado

Estas opções estão no painel, abaixo de **Utilizar caracteres universais**. Cada uma altera a forma como o JeffWrite lê as palavras em **Localizar**.

- **Soa como** encontra palavras que se pronunciam da mesma forma que a sua palavra. Atribui a cada palavra um código a partir dos seus sons, e encontra as palavras com o mesmo código. Escreva **Smith**, e encontra também "Smyth".
- **Localizar todas as formas da palavra** encontra todas as formas da palavra. Escreva **correr**, e encontra "corre," "correu" e "correndo". Funciona com palavras inteiras. Se escrever duas palavras, como **cão grande**, encontra-as lado a lado, em qualquer das suas formas.
- **Ignorar caracteres de pontuação** salta sinais como hífenes e pontos entre as letras. Escreva **email**, e encontra também "e-mail".
- **Ignorar caracteres de espaço em branco** salta espaços e tabulações entre as letras. Escreva **e mail**, e encontra "email".

Só uma de **Soa como**, **Localizar todas as formas da palavra** e **Utilizar caracteres universais** pode estar ativa de cada vez. **Soa como** e **Localizar todas as formas da palavra** encontram sempre palavras inteiras, por isso **Localizar apenas palavras inteiras** e **Corresponder maiúsculas e minúsculas** ficam a cinzento.

### Especial

Clique em **Especial** para colocar um código em **Localizar**: **Marca de Parágrafo** (^p), **Carácter de Tabulação** (^t), **Qualquer Algarismo** (^#), **Qualquer Letra** (^$), **Espaço em Branco** (^w), **Quebra de Secção** (^b), **Campo** (^d), **Marca de Nota de Rodapé** (^f), **Marca de Nota de Fim** (^e), ou **Carácter Circunflexo** (^^). Os códigos funcionam sem **Utilizar caracteres universais**. Uma **quebra de secção** é a marca que termina uma secção. Um **campo** é um pedaço de texto que o programa calcula, como um número de página ou uma data, e **^d** encontra onde cada um começa. As marcas de nota são os pequenos números no texto que apontam para uma nota de rodapé ou de fim. Estas quatro são encontradas, mas **Substituir** e **Substituir Tudo** não lhes tocam, porque alterar uma delas faria perder aquilo que representa. Os restantes sinais da caixa, como **?** e **\***, são então apenas esses caracteres.

### Formatação

Clique em **Formatação** para localizar apenas texto com um determinado aspeto: **Negrito**, **Itálico**, **Sublinhado**, **Rasurado**, **Realçado**, um **Tipo de letra**, ou um **Estilo** de parágrafo. O painel lista os aspetos que escolheu abaixo dos botões. Escolha **Sem formatação** para os limpar.

Pode deixar **Localizar** vazio. Então, cada troço de texto com esse aspeto é uma correspondência. Com palavras em **Localizar**, só são encontradas as palavras com esse aspeto. **Substituir Tudo** altera apenas essas palavras.

## Restringir a pesquisa

- **Corresponder maiúsculas e minúsculas** encontra apenas palavras com as mesmas letras maiúsculas. Com esta opção ativa, "Plano" não encontra "plano".
- **Localizar apenas palavras inteiras** ignora palavras mais longas que contenham a sua. Com esta opção ativa, "plano" não encontra "planeta".
- **Pesquisar** define em que sentido **Localizar seguinte** avança a partir do cursor. **Para baixo** vai até ao fim do documento, e **Para cima** vai até ao início. Cada uma para aí e indica-o. **Tudo** continua até à outra extremidade e é a escolha habitual. **Substituir Tudo** altera sempre o documento inteiro.

## Substituir palavras

1. Prima **Ctrl+H**, ou clique em **Base > Edição > Substituir**. O cursor vai para **Substituir por**.
2. Escreva as palavras novas em **Substituir por**. Para remover as palavras encontradas, deixe o campo vazio.
3. Escreva as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira correspondência. Clique em **Substituir** novamente para a alterar. O JeffWrite passa então para a correspondência seguinte. Cada alteração é um passo, por isso **Anular** (Ctrl+Z) desfaz uma de cada vez.
5. Clique em **Substituir Tudo** para alterar todas as correspondências de uma só vez. O painel indica quantas alterou. **Substituir Tudo** altera também as palavras em caixas de texto e formas, e é um só passo, por isso um único **Anular** repõe todas as palavras. Quando o **Controlo de Alterações** está ativo, cada substituição fica registada como uma alteração controlada, no texto e nas caixas. Uma caixa demasiado grande para ser alterada é contada na mensagem. **Substituir Tudo** não chega a caixas de texto em cabeçalhos e rodapés, a formas agrupadas, nem a caixas ligadas entre si. **Localizar seguinte** ainda não procura em caixas de texto.

As palavras novas assumem as maiúsculas da primeira letra das palavras que substituem. Se substituir uma palavra em negrito, a nova palavra fica em negrito.

Por exemplo, para mudar "color" para "colour" em todo o lado: escreva **color** em **Localizar**, escreva **colour** em **Substituir por**, e clique em **Substituir Tudo**.

## O que a pesquisa analisa

- Analisa o texto do documento, incluindo o texto das tabelas. Um marcador no meio de algumas palavras, que alguns ficheiros têm, não impede que essas palavras sejam encontradas. **Substituir** e **Substituir Tudo** mantêm o marcador.
- **Localizar seguinte** procura primeiro no texto. Depois passa para os cabeçalhos, em seguida para os rodapés, depois para as notas de rodapé, e por fim volta ao início do texto. Quando uma correspondência está numa delas, o JeffWrite abre-a para edição e seleciona a correspondência. O painel indica onde está, por exemplo "Encontrado no cabeçalho da página 3." **Substituir** troca a correspondência em que está, no texto ou num cabeçalho, rodapé ou nota de rodapé. **Substituir Tudo** altera todas as correspondências em todos eles, e um único **Anular** as repõe. As notas de fim fazem parte do texto, por isso são procuradas com ele. Consulte [Cabeçalhos e rodapés](headers-footers).
- Ignora texto definido como **Oculto**, porque esse texto não é mostrado. Consulte [Alterar o aspeto do texto](font).

## Consulte também

- [Alterar o aspeto do texto](font)
- [Utilizar o JeffWrite com o teclado](keyboard)
