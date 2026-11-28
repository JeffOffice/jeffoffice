# Guia Revisão
<!-- keywords: guia revisão, comparar, painel de revisão, aceitar, rejeitar, faixa de opções, revisão de texto, ortografia, dicionário de sinônimos, contagem de palavras, comentários, novo comentário, excluir comentário, anterior, próximo, mostrar comentários, idioma, idioma de revisão, definir idioma, não verificar ortografia, traduzir, tradução, libretranslate, gramática, verificação gramatical, estilo, linha azul, linha pontilhada, verificar gramática enquanto você digita -->

A guia **Revisão** verifica suas palavras, traduz o texto e gerencia os comentários.

- **Revisão de Texto** tem **Ortografia** (F7), **Dicionário de Sinônimos** (Shift+F7), **Contar Palavras** e **Verificar Acessibilidade**. Veja [Digitar e formatar texto](text).
- **Idioma** tem **Traduzir**, **Definir Idioma de Revisão de Texto** e **Não Verificar Ortografia**. Veja abaixo.
- **Comparar** tem **Comparar**, **Aceitar**, **Rejeitar**, **Anterior**, **Próximo** e **Painel de Revisão**. Veja [Comparar apresentações](compare).
- **Comentários** tem **Novo Comentário** (Ctrl+Alt+M), **Excluir** com seu menu, **Anterior**, **Próximo** e **Mostrar Comentários**. Veja [Comentários](comments).

## Verificar gramática enquanto você digita

O JeffDeck também procura erros claros de gramática e de estilo no texto, nos slides e nas anotações do orador. Um problema recebe uma linha pontilhada azul embaixo, e uma palavra com erro de ortografia mantém sua linha vermelha ondulada. Clique com o botão direito no problema para ver o que está errado e escolher uma sugestão, ou escolha **Ignorar Este Problema** ou **Ignorar Tudo**. **Ortografia** (F7) passa para os problemas de gramática dos slides depois da ortografia. Texto em um idioma que não é verificado, e texto marcado como "não verificar", é ignorado. Para desativar, clique em **Arquivo > Opções > Revisão de Texto** e desmarque **Verificar gramática enquanto você digita**. É uma única opção para todos os aplicativos do JeffOffice.

**Quais idiomas são verificados.** A gramática é verificada somente em inglês, em todas as variantes regionais (americano, britânico, canadense, australiano e os demais). Texto em qualquer outro idioma é verificado apenas quanto à ortografia e não recebe marcas de gramática. Um parágrafo é verificado no idioma em que está marcado, e texto sem idioma é verificado como inglês. Em um parágrafo com mais de um idioma, somente o inglês é verificado e o restante é ignorado. Texto que um arquivo marca como "não verificar" também é ignorado.

**Escolha o que é verificado.** Clique em **Arquivo > Opções > Revisão de Texto > Configurações de gramática** para escolher os tipos de problema: **Gramática** (verbos que não concordam, forma errada de um verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífens extras ou ausentes), **Maiúsculas**, **Palavras trocadas** (palavras que soam parecido), **Repetição** e **Estilo**. **Estilo** fica desativado até que você o ative. Ele faz sugestões, não aponta erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam com a mesma palavra e expressões prolixas ("in order to", "due to the fact that"). A escolha é mantida ao clicar em **OK** nas Opções. Ela se aplica de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas deste aplicativo, e aos demais aplicativos do JeffOffice na próxima vez que forem iniciados.

## Definir o idioma de um texto

A ortografia verifica cada trecho de texto no próprio idioma. Para mudar o idioma de algumas palavras, selecione-as, ou selecione uma caixa de texto ou um espaço reservado inteiro, e clique em **Definir Idioma de Revisão de Texto** no grupo **Idioma**. Escolha o idioma na lista e clique em **OK**. As palavras nesse idioma são verificadas com o dicionário desse idioma, e o arquivo mantém a escolha ao ser salvo como apresentação ou apresentação OpenDocument. Texto que você não marcou é verificado em inglês americano.

## Não verificar alguns textos

Alguns textos não devem ser verificados: um código, um nome ou uma linha em um idioma inventado. Selecione-o e clique em **Não Verificar Ortografia** no grupo **Idioma**. As linhas vermelhas onduladas desaparecem. Clique novamente para verificar o texto outra vez. A caixa de anotações guarda apenas texto simples, por isso não tem idioma próprio.

## Traduzir texto

1. Selecione as palavras em uma caixa de texto ou nas anotações. Para traduzir todo o texto de uma caixa de texto, selecione a caixa.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual traduzir.
4. Clique em **OK**.

Em seguida, seu navegador da web abre um serviço de tradução chamado LibreTranslate, com o seu texto. Nada é enviado até que você clique em **OK**. Você pode alterar o endereço do serviço na caixa, por exemplo para um que você mesmo hospede. Somente endereços da web seguros (https) são usados, e endereços http para este computador. São enviadas no máximo 5.000 letras.

## Verificar acessibilidade

**Verificar Acessibilidade** abre uma pequena janela que lista o que pode ser difícil de usar para algumas pessoas: uma imagem ou forma sem texto alternativo, um slide sem título, uma tabela sem linha de cabeçalho, uma tabela com células mescladas, um vídeo sem legendas, um slide cujo título é igual ao de outro slide (uma dica), um link cujo texto é o próprio endereço ou não diz para onde leva (como "aqui" ou "leia mais"), texto difícil de ler sobre o seu fundo, e objetos que um leitor de tela lê em uma ordem diferente da que a posição sugere. O texto precisa de contraste de 4,5 para 1 com o fundo (3 para 1 para textos de 18 pontos ou mais, ou de 14 pontos ou mais em negrito); quando o texto está sobre uma imagem, uma dica pede que você confira o contraste a olho. Um leitor de tela lê os objetos de um slide do fundo da pilha para a frente. Clique em **Ordem de Leitura** para vê-los nessa ordem, e use **Ler Antes** e **Ler Depois** (ou arraste) para alterá-la; cada movimento é um passo de Desfazer. Um leitor de tela usa a primeira linha de uma tabela para nomear suas colunas: selecione um problema de tabela e clique em **Usar a primeira linha como cabeçalho** para ativar a linha de cabeçalho (um passo de Desfazer). Um vídeo sem legendas aparece como dica, pois vídeos precisam de legendas para pessoas que não podem ouvi-los. Não há correção com um clique: selecione o vídeo e use **Inserir Legendas** na guia **Reprodução**, e a dica desaparece. Clique em um problema para ir ao seu slide e selecionar o objeto. Dê um texto alternativo à imagem no painel **Formatar Imagem**, ou marque-a como decorativa, e depois clique em **Verificar Novamente**. Você também pode iniciar a verificação em **Arquivo > Informações**.

## Relacionados

- [Comparar apresentações](compare)
- [Comentários](comments)
- [Digitar e formatar texto](text)
