# Verifique sua ortografia
<!-- keywords: ortografia, verificar ortografia, corretor ortográfico, palavra errada, linha vermelha, linha ondulada, sugestões, ignorar, ignorar tudo, adicionar ao dicionário, dicionário personalizado, dicionário, idioma, revisão de texto, f7, revisão, verificar ortografia ao digitar, maiúsculas, números, gramática, verificar gramática, estilo, linha azul, linha pontilhada, verificar gramática ao digitar, erro ortográfico, sublinhado vermelho, palavras -->

O JeffWrite verifica sua ortografia enquanto você digita. Uma palavra que não está no dicionário recebe uma linha vermelha ondulada embaixo. A verificação é feita no seu próprio computador, sem conexão com a internet, e conhece 31 idiomas e variantes regionais.

## Enquanto você digita

- Uma linha vermelha ondulada significa que o dicionário não tem a palavra. A palavra pode estar certa mesmo assim. Nomes e palavras novas costumam faltar no dicionário. Os nomes dos aplicativos do JeffOffice (JeffWrite, JeffPub, JeffDeck, JeffOffice, JeffCrunch, JeffMail e JeffBase) e "LLC" nunca são marcados, em nenhum idioma, com ou sem "s" no final.
- A palavra que você está digitando não é verificada até você passar para outra, para que uma palavra ainda incompleta não fique piscando.
- As linhas aparecem apenas na tela. Elas nunca são impressas e nunca vão para um PDF.

Para corrigir uma palavra, **clique com o botão direito** sobre ela. O menu mostra estas opções acima das usuais:

- Até seis **sugestões**, a melhor primeiro, em negrito. Clique em uma para colocá-la no lugar da palavra. É uma única etapa, então **Ctrl+Z** a desfaz.
- **Ignorar Tudo** deixa essa palavra em paz, em qualquer lugar em que apareça, até você fechar o JeffWrite.
- **Adicionar ao Dicionário** ensina a palavra ao JeffWrite de forma permanente. Ela é guardada em um pequeno arquivo na pasta do JeffOffice no seu computador, e todos os aplicativos do JeffOffice a usam. Para ver ou alterar sua lista de palavras, veja Suas próprias palavras, mais abaixo.

Se o dicionário não tiver nenhuma sugestão, o menu informa isso.

## Percorrer os erros de ortografia

Pressione **F7**, ou clique em **Revisão > Ortografia**. O painel Ortografia abre à direita. Ele mostra a primeira palavra com erro depois do cursor, selecionada no texto, com a linha em que ela está e uma lista de sugestões.

- **Alterar** coloca a sugestão escolhida no lugar da palavra. Clique duas vezes em uma sugestão para fazer o mesmo.
- **Alterar Tudo** faz isso para cada cópia da palavra nos lugares por onde o painel passa, listados abaixo. É uma única etapa, então **Ctrl+Z** desfaz todas elas, onde quer que estivessem. Ao terminar, o painel informa quantas palavras foram alteradas e em quantos lugares. O texto, cada caixa de texto, cada nota e cada cabeçalho ou rodapé contam como um lugar cada.
- **Ignorar** deixa esta palavra de lado. Sua linha vermelha permanece.
- **Ignorar Tudo** e **Adicionar ao Dicionário** funcionam como no menu do botão direito.

O painel começa onde está o cursor: no texto, ou na caixa de texto, nota de rodapé, cabeçalho ou rodapé que você está editando. Ele percorre primeiro o texto do documento, inclusive as tabelas, depois as caixas de texto, depois as notas de rodapé e por último os cabeçalhos e rodapés. Quando uma palavra está em uma caixa de texto, nota de rodapé, cabeçalho ou rodapé, o JeffWrite abre esse local para edição, como faz a Localizar, para que você veja a palavra onde ela está. Um cabeçalho ou rodapé que nenhuma página exibe é ignorado, e **Alterar Tudo** também o deixa como está. A partir do cursor, o painel vai até o fim desse local, depois segue pelos outros nesta ordem e recomeça do início, até o ponto em que começou. **Alterar** é uma única etapa que pode ser desfeita, e **Alterar Tudo** procura em todos os lugares por onde o painel passa, também como uma única etapa. Uma palavra em uma nota de fim pode ser ignorada ou adicionada, mas não alterada. Quando não restam mais palavras, o painel informa que a verificação ortográfica foi concluída. Quando a ortografia termina, ele passa aos problemas de gramática e estilo (veja Verificar gramática, mais abaixo).

## Suas próprias palavras

Clique em **Arquivo > Opções > Revisão de Texto > Dicionário Personalizado** para ver e alterar suas palavras. A caixa de diálogo tem duas listas, cada uma com uma caixa **Localizar** para filtrá-la:

- **Palavras Adicionadas** são palavras que você ensinou ao JeffWrite. Elas são aceitas em todos os idiomas.
- **Palavras Excluídas** são palavras que você quer ver marcadas como erradas, mesmo que um dicionário as aceite, como uma palavra que você sempre digita errado. Elas são marcadas onde quer que apareçam, com qualquer uso de maiúsculas, e nunca são oferecidas como sugestão. Uma palavra excluída vale apenas para aquela forma exata, não para as outras: excluir "casa" não marca "casas". Adicionar uma palavra ao seu dicionário, pelo painel, pelo menu do botão direito ou pela caixa de diálogo, a tira da lista de excluídas. **Ignorar Tudo** ainda funciona sobre uma palavra excluída até você fechar o programa.

Digite uma palavra e clique em **Adicionar**, ou selecione palavras e clique em **Remover**. Uma palavra fica em uma lista de cada vez: adicioná-la a uma lista a tira da outra. **Importar** lê as palavras de um arquivo de texto simples, com uma palavra por linha, e **Exportar** grava a lista no mesmo tipo de arquivo (UTF-8). Uma linha que tenha espaço ou caractere de controle, ou que tenha mais de 100 letras, é deixada de fora, e a caixa de diálogo informa quantas foram. Uma lista pode ter até 1 MB de texto. Nada muda até você clicar em **OK**. As listas são suas, são as mesmas em todos os aplicativos do JeffOffice e nunca são salvas em um documento.

## Quando a Autocorreção altera uma palavra

Quando a Autocorreção altera uma palavra enquanto você digita, um pequeno botão **Opções de Autocorreção** aparece abaixo do cursor. Ele tem três opções. Um leitor de tela informa que há opções quando o botão aparece, e a descrição do botão diz como abri-las. Você também pode pressionar **Alt+Shift+F10** para abri-las. É o comando **Menu de Opções de Autocorreção**, que fica na página Teclado das opções junto com os demais comandos, e você pode atribuir a ele outra tecla ali.

- **Desfazer Correção Automática** restaura a palavra como você a digitou. É uma única etapa, então **Refazer** aplica a alteração novamente.
- Uma opção que interrompe a alteração. Depende do que alterou a palavra: **Parar de Substituir** remove a entrada da sua lista de substituições, **Manter Como Digitado** acrescenta às palavras que mantêm as maiúsculas uma palavra com duas letras maiúsculas ("THursday"), e **Parar de Usar Maiúscula** desativa a maiúscula dos nomes dos dias, da primeira letra das frases ou da primeira letra das células de tabela em todos os documentos, não só nesta palavra, e informa isso (é a mesma chave da caixa de diálogo Autocorreção). Ela também restaura a palavra como você a digitou.
- **Opções de Autocorreção...** abre a caixa de diálogo Autocorreção.

O botão desaparece quando você digita, edita o texto ou move o cursor.

## Ativar ou desativar as linhas vermelhas

Clique em **Revisão > Verificar Ortografia ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**, e é a única opção que todos os aplicativos do JeffOffice usam. Ali você também pode escolher ignorar palavras em MAIÚSCULAS e palavras com números, como 3.º. Ambas são ignoradas no início.

## Verificar gramática ao digitar

O JeffWrite também procura erros claros de gramática e estilo no texto. Um problema recebe uma linha azul pontilhada embaixo. A ortografia mantém a linha vermelha ondulada, e uma palavra com erro de ortografia nunca é marcada como problema de gramática. Estes são os erros que ele procura:

- uma palavra repetida ("the the")
- "a" e "an" antes do som errado ("an test", "a hour")
- uma frase que começa com letra minúscula depois de um ponto
- "its" e "it's", "their" e "there", "your" e "you're", "then" e "than", e "lose" e "loose", quando as palavras ao redor deixam o erro claro
- "could of" no lugar de "could have"
- dois espaços seguidos dentro de uma frase
- um espaço antes de vírgula, ponto ou outro sinal que pertence à palavra anterior
- falta de espaço depois do fim de uma frase
- "alot", e "everyday" no lugar de "every day" ("almost everyday")
- verbos que não concordam com o sujeito em casos claros ("she don't", "they was", "it have", "everyone are", "there is many reasons")
- a forma errada de um verbo ("did went", "has went", "can't hardly", "don't have no")
- palavras confundidas ("who's car", "an affect on", "less people", "irregardless", "can't breath", "weather" no lugar de "whether", "chose" no lugar de "choose", "advise" no lugar de "advice")
- "between you and I", "Me and him went", "more better" e "these thing"
- uma vírgula depois do ano de uma data ("On June 5, 2020, we left"), uma vírgula depois de cidade e estado ("in Austin, Texas, the heat") e um hífen em um composto antes de um substantivo ("a well-known author")
- um "i" minúsculo no lugar de "I", e os dias e meses em letra minúscula

Ele prefere deixar passar um erro a marcar um texto que está certo, então uma frase sem linha azul não é garantia de que esteja correta.

Clique com o botão direito sobre o problema. O menu mostra o que está errado e depois uma ou mais sugestões. Clique em uma sugestão para colocá-la no texto, e pressione Ctrl+Z para desfazer. **Ignorar Este Problema** remove a linha apenas deste lugar, até você fechar o JeffWrite. As mesmas palavras em outro lugar mantêm a linha, e editar outra parte do parágrafo não a faz voltar. **Ignorar Tudo** remove a linha de todos os problemas com esta mensagem nestas palavras.

A verificação é feita no seu próprio computador, sem conexão com a internet, em segundo plano: a digitação nunca é interrompida, e um parágrafo é verificado um momento depois que você para de digitar nele. Um parágrafo é verificado até os seus primeiros 100.000 caracteres. Um documento que exige mais verificação do que o limite deixa de ser verificado, e a barra de status informa isso uma vez; a ortografia continua funcionando.

**Quais idiomas são verificados.** Inglês, francês, alemão e espanhol, cada um com regras próprias. O francês verifica o espaço sem quebra antes de ; : ! ? e dentro de « », o apóstrofo em palavras como "l'ami" e "qu'il", e confusões como "a" e "à". O alemão verifica "das" e "dass", "seid" e "seit", "wider" e "wieder", a vírgula antes de "dass" e "weil", e as maiúsculas em substantivos comuns. O espanhol procura o ¿ ou o ¡ que faltam, acentos em palavras interrogativas, "a ver" e "haber", e "del" e "al". Um parágrafo é verificado no idioma em que está marcado, e um texto sem idioma é verificado como inglês. Em um parágrafo com mais de um idioma, usa-se o primeiro idioma verificado e o restante é ignorado. Texto que um arquivo marca como "não verificar" também é ignorado. A gramática é verificada no texto do documento, e ainda não nas tabelas, cabeçalhos, rodapés ou notas.

Para desativá-la, clique em **Revisão > Verificar Gramática ao Digitar**. A mesma opção está em **Arquivo > Opções > Revisão de Texto**. É uma única opção para todos os aplicativos do JeffOffice, e vale igual em todas as janelas. O painel Ortografia (F7) percorre os problemas de gramática depois da ortografia, quando esta opção está ativada.

**Escolha o que ela procura.** Clique em **Arquivo > Opções > Revisão de Texto > Configurações de gramática** para escolher os tipos de problema: **Gramática** (verbos que não concordam, forma errada de verbo, "a" e "an"), **Pontuação** (espaços, vírgulas e hífens extras ou ausentes), **Maiúsculas e minúsculas**, **Palavras confundidas** (palavras que soam parecidas), **Repetição** e **Estilo**. **Estilo** fica desativado até você ativá-lo. Ele sugere melhorias, e não trata de erros: frases com mais de 40 palavras, a voz passiva, três frases seguidas que começam com a mesma palavra e expressões prolixas ("in order to", "due to the fact that"). A escolha é mantida quando você clica em **OK** nas Opções. Ela vale de imediato para as linhas azuis e para o painel Ortografia (F7) nas janelas abertas deste aplicativo, e para os outros aplicativos do JeffOffice na próxima vez que forem iniciados.

## Idiomas

O JeffWrite verifica cada trecho de texto no seu próprio idioma. Um arquivo pode indicar em que idioma está o texto, até uma única palavra, e o JeffWrite lê essa informação. Um texto sem indicação é verificado no idioma do documento. O arquivo informa esse idioma para o documento inteiro, e um documento novo usa inglês americano.

Texto em um idioma para o qual o JeffWrite não tem dicionário não é verificado. Também não é verificado o texto que um arquivo marca como "não verificar ortografia". Você ainda não pode alterar o idioma de um texto no JeffWrite, mas ele mantém o que o arquivo indica.

Os 31 dicionários são: inglês (americano, britânico, canadense e australiano), espanhol (21 regiões, incluindo México e Espanha), francês, alemão, italiano, holandês e português (Brasil e Portugal).

## O que ainda não é verificado

As linhas vermelhas aparecem no texto, nas células de tabelas e em cabeçalhos, rodapés ou notas de rodapé enquanto você os edita. Elas ainda não aparecem nas caixas de texto, mas o painel Ortografia percorre essas caixas.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
