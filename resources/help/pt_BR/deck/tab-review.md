# Guia Revisão
<!-- keywords: guia revisão, faixa de opções, revisão de texto, ortografia, dicionário de sinônimos, contagem de palavras, comentários, novo comentário, excluir comentário, anterior, próximo, mostrar comentários, idioma, idioma de revisão, definir idioma, não verificar ortografia, traduzir, tradução, libretranslate, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática enquanto você digita -->

A guia **Revisão** verifica suas palavras, traduz o texto e gerencia os comentários.

- **Revisão de Texto** tem **Ortografia** (F7), **Dicionário de Sinônimos** (Shift+F7), **Contar Palavras** e **Verificar Acessibilidade**. Veja [Digitar e formatar texto](text).
- **Idioma** tem **Traduzir**, **Definir Idioma de Revisão de Texto** e **Não Verificar Ortografia**. Veja abaixo.
- **Comentários** tem **Novo Comentário** (Ctrl+Alt+M), **Excluir** com seu menu, **Anterior**, **Próximo** e **Mostrar Comentários**. Veja [Comentários](comments).

## Verificar gramática enquanto você digita

O JeffDeck também procura erros claros de gramática e de estilo no texto, nos slides e nas anotações do orador. Um problema recebe uma linha pontilhada azul embaixo, e uma palavra escrita errada mantém sua linha ondulada vermelha. Clique com o botão direito no problema para ver o que está errado e escolher uma sugestão, ou escolha **Ignorar Esta Ocorrência** ou **Ignorar Tudo**. **Ortografia** (F7) passa, depois da ortografia, pelos problemas de gramática dos slides. Texto em um idioma que não é verificado, e texto marcado como "não verificar", é ignorado. Para desativar, clique em **Arquivo > Opções > Revisão de Texto** e desmarque **Verificar gramática enquanto você digita**. É uma única opção para todos os aplicativos do JeffOffice.

**Quais idiomas são verificados.** Inglês, francês, alemão e espanhol, cada um com regras próprias. O francês verifica o espaço sem quebra antes de ; : ! ? e dentro de « », o apóstrofo em palavras como "l'ami" e "qu'il", e confusões como "a" e "à". O alemão verifica "das" e "dass", "seid" e "seit", "wider" e "wieder", a vírgula antes de "dass" e "weil", e letras maiúsculas em substantivos comuns. O espanhol procura um ¿ ou ¡ que falte, acentos em palavras interrogativas, "a ver" e "haber", e "del" e "al". Um parágrafo é verificado no idioma em que está marcado, e texto sem idioma é verificado como inglês. Em um parágrafo com mais de um idioma, usa-se o primeiro idioma verificado e o restante é ignorado.

**Escolha o que é verificado.** Clique em **Arquivo > Opções > Revisão de Texto > Configurações de gramática** para escolher os tipos de problema: **Gramática** (verbos que não concordam, forma errada de um verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífens extras ou ausentes), **Maiúsculas e minúsculas**, **Palavras trocadas** (palavras que soam parecido), **Repetição** e **Estilo**. **Estilo** fica desativado até que você o ative. Ele faz sugestões, e não trata de erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam com a mesma palavra, e expressões prolixas ("in order to", "due to the fact that"). A escolha é mantida ao clicar em **OK** em Opções. Ela se aplica de imediato às linhas azuis e ao painel Ortografia (F7) nas janelas abertas deste aplicativo, e aos outros aplicativos do JeffOffice na próxima vez que forem iniciados.

## Definir o idioma de um texto

A ortografia verifica cada trecho de texto em seu próprio idioma. Para mudar o idioma de algumas palavras, selecione-as, ou selecione uma caixa de texto ou um espaço reservado inteiro, e clique em **Definir Idioma de Revisão de Texto** no grupo **Idioma**. Escolha o idioma na lista e clique em **OK**. As palavras nesse idioma são verificadas com o dicionário desse idioma, e o arquivo mantém a escolha ao ser salvo como apresentação ou apresentação OpenDocument. Texto que você não marcou é verificado em inglês americano.

## Não verificar alguns textos

Alguns textos não devem ser verificados: um código, um nome ou uma linha em um idioma inventado. Selecione-o e clique em **Não Verificar Ortografia** no grupo **Idioma**. As linhas onduladas vermelhas desaparecem. Clique de novo para verificar o texto outra vez. A caixa de anotações guarda apenas texto simples, por isso não tem idioma próprio.

## Traduzir texto

1. Selecione as palavras em uma caixa de texto ou nas anotações. Para traduzir tudo em uma caixa de texto, selecione a caixa.
2. Clique em **Traduzir** no grupo **Idioma**.
3. Escolha o idioma para o qual traduzir.
4. Clique em **OK**.

Em seguida, seu navegador da web abre um serviço de tradução chamado LibreTranslate, com seu texto nele. Nada é enviado até que você clique em **OK**. Você pode alterar o endereço do serviço na caixa, por exemplo para um que você mesmo hospede. Somente endereços seguros (https) são usados, e endereços http para este computador. São enviadas no máximo 5.000 letras.

## Verificar acessibilidade

**Verificar Acessibilidade** abre uma pequena janela que lista o que pode ser difícil de usar para algumas pessoas: uma imagem ou uma forma sem texto alternativo, um slide sem título, uma tabela sem linha de cabeçalho, um vídeo sem legendas, texto difícil de ler sobre o seu fundo, e objetos que um leitor de tela lê em uma ordem diferente da que a posição deles sugere. O texto precisa de um contraste de 4,5 para 1 com o seu fundo (3 para 1 para texto de 18 pontos ou mais, ou de 14 pontos ou mais em negrito); quando o texto está sobre uma imagem, uma dica pede que você confira o contraste a olho. Um leitor de tela lê os objetos de um slide do fundo da pilha para a frente. Clique em **Ordem de Leitura** para vê-los nessa ordem, e use **Mover Antes** e **Mover Depois** (ou arraste) para alterá-la; cada movimento é uma etapa de Desfazer. Um leitor de tela usa a primeira linha de uma tabela para dar nome às colunas: selecione um problema de tabela e clique em **Usar a primeira linha como cabeçalho** para ativar a linha de cabeçalho (uma etapa de Desfazer). Um vídeo sem legendas aparece como uma dica: vídeos precisam de legendas para pessoas que não podem ouvi-los. Não há correção com um clique; selecione o vídeo e use **Inserir Legendas** na guia **Reprodução**, e a dica desaparece. Clique em um problema para ir ao seu slide e selecionar o objeto. Dê à imagem um texto alternativo no painel **Formatar Imagem**, ou marque-a como decorativa, e depois clique em **Verificar Novamente**. Você também pode iniciar a verificação em **Arquivo > Informações**.

## Relacionados

- [Comentários](comments)
- [Digitar e formatar texto](text)
