# Localizar e substituir palavras
<!-- keywords: localizar avançado, soa como, soundex, formas da palavra, todas as formas de uma palavra, ignorar pontuação, ignorar espaços, especial, marca de parágrafo, tabulação, localizar por formatação, formatação, negrito, fonte, estilo, curingas, curinga, padrão, expressão regular, grupos, contagem de correspondências, realçar todas as correspondências, substituir, substituir tudo, pesquisar, procurar, alterar palavras, diferenciar maiúsculas de minúsculas, somente palavras inteiras, localizar próxima, ctrl+f, ctrl+h, edição, painel, texto oculto, desfazer -->

Use Localizar para ir até palavras no seu documento. Use Substituir para alterá-las. Os dois funcionam em um só painel à direita da janela.

## Localizar palavras

1. Pressione **Ctrl+F** ou clique em **Página Inicial > Edição > Localizar**. O painel **Localizar e Substituir** é aberto, com o cursor em **Localizar**.
2. Digite as palavras que você deseja. Depois pressione **Enter** ou clique em **Localizar Próxima**.

O JeffWrite começa onde está o cursor. Ele seleciona o próximo lugar em que encontra as suas palavras e rola as páginas para que você possa vê-lo. Esse lugar é chamado de **correspondência**. Clique em **Localizar Próxima** de novo para ir à próxima correspondência. Depois da última, ele volta ao início do documento.

Enquanto você digita, o painel conta as correspondências, como "9 correspondências", e cada uma recebe uma faixa amarela nas páginas. As faixas aparecem também em células de tabelas, cabeçalhos, rodapés e notas de rodapé. Depois de **Localizar Próxima**, o painel informa em qual você está, como "Correspondência 2 de 4". A contagem e as faixas mudam quando você marca **Diferenciar maiúsculas de minúsculas** ou **Localizar apenas palavras inteiras**, ou quando altera o texto. Elas desaparecem quando você fecha o painel.

## Localizar com curingas

Marque **Usar curingas** para pesquisar um padrão, e não apenas palavras. Um **curinga** é um sinal que representa outras letras. Estes são os sinais:

- **?** representa qualquer uma letra. **c?t** encontra "cat" e "cot".
- **\*** representa qualquer sequência de letras. **c\*t** encontra "cat" e "cart".
- **<** é o início de uma palavra, e **>** é o fim. **<the>** encontra "the", mas não "other".
- **[abc]** é qualquer uma daquelas letras, **[a-c]** é qualquer letra de a a c, e **[!abc]** é qualquer letra que não seja essas.
- **{2}** repete duas vezes a letra anterior. **{1,3}** repete de uma a três vezes.
- **@** significa uma ou mais ocorrências da letra anterior.
- **\\** antes de um sinal localiza o próprio sinal. **\\?** localiza um ponto de interrogação.
- **^p** é uma marca de parágrafo, **^t** é uma tabulação, **^#** é um dígito, **^$** é uma letra e **^w** é um espaço em branco.

Coloque parte do padrão entre **( )** para criar um **grupo**. Em **Substituir por**, **\1** é o primeiro grupo, **\2** é o segundo, e assim até **\9**. **^&** é tudo o que foi encontrado. Por exemplo, localize **(<[A-Z]@>), (<[A-Z]@>)** e substitua por **\2 \1** para transformar "Silva, João" em "João Silva". **Localizar apenas palavras inteiras** fica desativada enquanto os curingas estão ligados, porque **<** e **>** fazem esse trabalho.

Se o padrão tiver algum erro, o painel informa qual é.

Se as suas palavras não estiverem no documento, o painel avisa, e nada se move.

Você pode continuar trabalhando no documento enquanto o painel está aberto. Clique na página para digitar. Clique no **X** no alto do painel para fechá-lo.

## Localizar avançado

Estas opções estão no painel, abaixo de **Usar curingas**. Cada uma muda a forma como o JeffWrite lê as palavras em **Localizar**.

- **Soa como** encontra palavras pronunciadas da mesma forma que a sua palavra. Ele atribui a cada palavra um código baseado nos sons e encontra as palavras com o mesmo código. Digite **Silva**, e ele também encontra "Silvia".
- **Localizar todas as formas da palavra** encontra todas as formas da palavra. Digite **correr**, e ele encontra "corre," "correndo" e "corri". Funciona com palavras inteiras. Se você digitar duas palavras, como **cachorro grande**, ele as encontra lado a lado, em qualquer uma de suas formas.
- **Ignorar caracteres de pontuação** pula sinais como hífens e pontos entre as letras. Digite **email**, e ele também encontra "e-mail".
- **Ignorar caracteres de espaço em branco** pula espaços e tabulações entre as letras. Digite **e mail**, e ele encontra "email".

Apenas uma entre **Soa como**, **Localizar todas as formas da palavra** e **Usar curingas** pode estar ligada por vez. **Soa como** e **Localizar todas as formas da palavra** sempre encontram palavras inteiras, por isso **Localizar apenas palavras inteiras** e **Diferenciar maiúsculas de minúsculas** ficam desativadas.

### Especial

Clique em **Especial** para colocar um código em **Localizar**: **Marca de Parágrafo** (^p), **Caractere de Tabulação** (^t), **Qualquer Dígito** (^#), **Qualquer Letra** (^$), **Espaço em Branco** (^w), **Quebra de Seção** (^b), **Campo** (^d), **Marca de Nota de Rodapé** (^f), **Marca de Nota de Fim** (^e) ou **Caractere de Circunflexo** (^^). Os códigos funcionam sem **Usar curingas**. Uma **quebra de seção** é a marca que termina uma seção. Um **campo** é um trecho de texto que o programa calcula, como um número de página ou uma data, e **^d** encontra onde cada um começa. As marcas de nota são os números pequenos no texto que apontam para uma nota de rodapé ou nota de fim. Esses quatro são encontrados, mas **Substituir** e **Substituir Tudo** não os alteram, porque mudar um deles faria perder aquilo que ele representa. Os demais sinais da caixa, como **?** e **\***, passam a ser apenas esses caracteres.

### Formatação

Clique em **Formatar** para encontrar somente texto com determinada aparência: **Negrito**, **Itálico**, **Sublinhado**, **Tachado**, **Realçado**, uma **Fonte** ou um **Estilo** de parágrafo. O painel lista as aparências escolhidas abaixo dos botões. Escolha **Sem Formatação** para limpá-las.

Você pode deixar **Localizar** vazio. Então, cada trecho de texto com essa aparência é uma correspondência. Com palavras em **Localizar**, somente as palavras com essa aparência são encontradas. **Substituir Tudo** altera apenas essas palavras.

## Restringir a pesquisa

- **Diferenciar maiúsculas de minúsculas** encontra somente palavras com as mesmas letras maiúsculas. Com a opção ligada, "Plano" não encontra "plano".
- **Localizar apenas palavras inteiras** ignora palavras maiores que contêm a sua. Com a opção ligada, "plano" não encontra "planejamento".

## Substituir palavras

1. Pressione **Ctrl+H** ou clique em **Página Inicial > Edição > Substituir**. O cursor vai para **Substituir por**.
2. Digite as palavras novas em **Substituir por**. Para remover as palavras encontradas, deixe o campo vazio.
3. Digite as palavras antigas em **Localizar**.
4. Clique em **Substituir** para ver a primeira correspondência. Clique em **Substituir** de novo para alterá-la. Em seguida, o JeffWrite passa para a próxima correspondência. Cada alteração é uma etapa, então **Desfazer** (Ctrl+Z) desfaz uma de cada vez.
5. Clique em **Substituir Tudo** para alterar todas as correspondências de uma vez. O painel informa quantas foram alteradas. **Substituir Tudo** é uma única etapa, então um único **Desfazer** devolve todas as palavras.

As palavras novas assumem a letra inicial das palavras que substituem. Se você substituir uma palavra em negrito, a nova palavra fica em negrito.

Por exemplo, para trocar "color" por "colour" em todo o documento: digite **color** em **Localizar**, digite **colour** em **Substituir por** e clique em **Substituir Tudo**.

## O que a pesquisa examina

- Ela examina o texto do documento, inclusive o texto das tabelas. Um indicador no meio de algumas palavras, que alguns arquivos têm, não impede que elas sejam encontradas. **Substituir** e **Substituir Tudo** mantêm o indicador.
- **Localizar Próxima** examina primeiro o texto. Depois passa para os cabeçalhos, em seguida para os rodapés, depois para as notas de rodapé e, por fim, volta ao início do texto. Quando uma correspondência está em um deles, o JeffWrite o abre para edição e seleciona a correspondência. O painel informa onde ela está, como "Encontrada no cabeçalho da página 3." **Substituir** troca a correspondência em que você está, seja no texto, em um cabeçalho, rodapé ou nota de rodapé. **Substituir Tudo** altera todas as correspondências em todos eles, e um único **Desfazer** traz todas de volta. As notas de fim fazem parte do texto, por isso são pesquisadas junto com ele. Veja [Cabeçalhos e rodapés](headers-footers).
- Ela ignora texto definido como **Oculto**, porque esse texto não é mostrado. Veja [Alterar a aparência do texto](font).

## Veja também

- [Alterar a aparência do texto](font)
- [Usar o JeffWrite com o teclado](keyboard)
